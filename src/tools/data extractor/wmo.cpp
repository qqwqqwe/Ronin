/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2014-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mpqfile.h"
#include "system.h"
#include "wmo.h"
#include "vec3d.h"

using namespace std;
extern uint16 *LiqType;

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08

#define MAP_LIQUID_TYPE_DARK_WATER  0x10
#define MAP_LIQUID_TYPE_WMO_WATER   0x20

extern VMAP::ModelSpawnMap modelSpawns;
extern VMAP::TiledModelSpawnMap tileModelSpawnSets;

WMORoot::WMORoot(std::string &filename)
    : filename(filename), col(0), nTextures(0), nGroups(0), nP(0), nLights(0),
    nModels(0), nDoodads(0), nDoodadSets(0), RootWMOID(0), liquidType(0)
{
    memset(bbcorn1, 0, sizeof(bbcorn1));
    memset(bbcorn2, 0, sizeof(bbcorn2));
}

bool WMORoot::open(HANDLE mpqarchive)
{
    MPQFile f(mpqarchive, filename.c_str());
    if(f.isEof ())
    {
        printf("No such file.\n");
        return false;
    }

    uint32 size;
    char fourcc[5];

    while (!f.isEof())
    {
        f.read(fourcc,4);
        f.read(&size, 4);

        flipcc(fourcc);
        fourcc[4] = 0;

        size_t nextpos = f.getPos() + size;

        if (!strcmp(fourcc,"MOHD")) // header
        {
            f.read(&nTextures, 4);
            f.read(&nGroups, 4);
            f.read(&nP, 4);
            f.read(&nLights, 4);
            f.read(&nModels, 4);
            f.read(&nDoodads, 4);
            f.read(&nDoodadSets, 4);
            f.read(&col, 4);
            f.read(&RootWMOID, 4);
            f.read(bbcorn1, 12);
            f.read(bbcorn2, 12);
            f.read(&liquidType, 4);
            break;
        }
        /*
        else if (!strcmp(fourcc,"MOTX"))
        {
        }
        else if (!strcmp(fourcc,"MOMT"))
        {
        }
        else if (!strcmp(fourcc,"MOGN"))
        {
        }
        else if (!strcmp(fourcc,"MOGI"))
        {
        }
        else if (!strcmp(fourcc,"MOLT"))
        {
        }
        else if (!strcmp(fourcc,"MODN"))
        {
        }
        else if (!strcmp(fourcc,"MODS"))
        {
        }
        else if (!strcmp(fourcc,"MODD"))
        {
        }
        else if (!strcmp(fourcc,"MOSB"))
        {
        }
        else if (!strcmp(fourcc,"MOPV"))
        {
        }
        else if (!strcmp(fourcc,"MOPT"))
        {
        }
        else if (!strcmp(fourcc,"MOPR"))
        {
        }
        else if (!strcmp(fourcc,"MFOG"))
        {
        }
        */
        f.seek((int)nextpos);
    }
    f.close ();
    return true;
}

bool WMORoot::ConvertToVMAPRootWmo(FILE* pOutfile)
{
    //printf("Convert RootWmo...\n");

    fwrite(szRawVMAPMagic, 1, 8, pOutfile);
    unsigned int nVectors = 0;
    fwrite(&nVectors,sizeof(nVectors), 1, pOutfile); // will be filled later
    fwrite(&nGroups, 4, 1, pOutfile);
    fwrite(&RootWMOID, 4, 1, pOutfile);
    return true;
}

WMOGroup::WMOGroup(const std::string &filename) :
    filename(filename), MOPY(0), MOVI(0), MoviEx(0), MOVT(0), MOBA(0), MobaEx(0),
    hlq(0), LiquEx(0), LiquBytes(0), groupName(0), descGroupName(0), mogpFlags(0),
    moprIdx(0), moprNItems(0), nBatchA(0), nBatchB(0), nBatchC(0), fogIdx(0),
    liquidType(0), groupWMOID(0), mopy_size(0), moba_size(0), LiquEx_size(0),
    nVertices(0), nTriangles(0), liquflags(0)
{
    memset(bbcorn1, 0, sizeof(bbcorn1));
    memset(bbcorn2, 0, sizeof(bbcorn2));
}

bool WMOGroup::open(HANDLE mpqarchive)
{
    MPQFile f(mpqarchive, filename.c_str());
    if(f.isEof ())
    {
        printf("No such file.\n");
        return false;
    }

    uint32 size;
    char fourcc[5];
    while (!f.isEof())
    {
        f.read(fourcc,4);
        fourcc[4] = 0;
        flipcc(fourcc);
        f.read(&size, 4);

        size_t nextpos = f.getPos() + size;
        if (!strcmp(fourcc,"MOGP"))//header
        {
            f.read(&groupName, 4);
            f.read(&descGroupName, 4);
            f.read(&mogpFlags, 4);
            f.read(bbcorn1, 12);
            f.read(bbcorn2, 12);
            f.read(&moprIdx, 2);
            f.read(&moprNItems, 2);
            f.read(&nBatchA, 2);
            f.read(&nBatchB, 2);
            f.read(&nBatchC, 4);
            f.read(&fogIdx, 4);
            f.read(&liquidType, 4);
            f.read(&groupWMOID,4);
            // Not sure what the next 8 bits are, but it's part of the header
            f.seekRelative(8);
            continue;
        }
        else if (!strcmp(fourcc,"MOPY"))
        {
            MOPY = new char[size];
            mopy_size = size;
            nTriangles = (int)size / 2;
            f.read(MOPY, size);
        }
        else if (!strcmp(fourcc,"MOVI"))
        {
            MOVI = new uint16[size/2];
            f.read(MOVI, size);
        }
        else if (!strcmp(fourcc,"MOVT"))
        {
            MOVT = new float[size/4];
            f.read(MOVT, size);
            nVertices = (int)size / 12;
        }
        else if (!strcmp(fourcc,"MONR"))
        {
        }
        else if (!strcmp(fourcc,"MOTV"))
        {
        }
        else if (!strcmp(fourcc,"MOBA"))
        {
            MOBA = new uint16[size/2];
            moba_size = size/2;
            f.read(MOBA, size);
        }
        else if (!strcmp(fourcc,"MLIQ"))
        {
            liquflags |= 1;
            hlq = new WMOLiquidHeader();
            f.read(hlq, 0x1E);
            LiquEx_size = sizeof(WMOLiquidVert) * hlq->xverts * hlq->yverts;
            LiquEx = new WMOLiquidVert[hlq->xverts * hlq->yverts];
            f.read(LiquEx, LiquEx_size);
            int nLiquBytes = hlq->xtiles * hlq->ytiles;
            LiquBytes = new char[nLiquBytes];
            f.read(LiquBytes, nLiquBytes);

            /* std::ofstream llog("Buildings/liquid.log", ios_base::out | ios_base::app);
            llog << filename;
            llog << "\nbbox: " << bbcorn1[0] << ", " << bbcorn1[1] << ", " << bbcorn1[2] << " | " << bbcorn2[0] << ", " << bbcorn2[1] << ", " << bbcorn2[2];
            llog << "\nlpos: " << hlq->pos_x << ", " << hlq->pos_y << ", " << hlq->pos_z;
            llog << "\nx-/yvert: " << hlq->xverts << "/" << hlq->yverts << " size: " << size << " expected size: " << 30 + hlq->xverts*hlq->yverts*8 + hlq->xtiles*hlq->ytiles << std::endl;
            llog.close(); */
        }
        f.seek((int)nextpos);
    }
    f.close();
    return true;
}

bool isCollidable(uint16 flag)
{
    if(flag & WMO_MATERIAL_COLLISION)
        return true;
    if((flag & WMO_MATERIAL_RENDER) && !(flag & WMO_MATERIAL_DETAIL))
        return true;
    // Collide hit sounds pretty obvious..
    if(flag & WMO_MATERIAL_COLLIDE_HIT);
    return false;
}

int WMOGroup::ConvertToVMAPGroupWmo(FILE *output, WMORoot *rootWMO, bool preciseVectorData)
{
    fwrite(&mogpFlags,sizeof(uint32),1,output);
    fwrite(&groupWMOID,sizeof(uint32),1,output);
    // group bound
    fwrite(bbcorn1, sizeof(float), 3, output);
    fwrite(bbcorn2, sizeof(float), 3, output);
    fwrite(&liquflags,sizeof(uint32),1,output);

    //-------GRP-------------------------------------
    static char GRP[] = "GRP ";
    fwrite(GRP,1,4,output);

    int nColTriangles = 0, k = 0;

    //-------MOBA------------------------------------
    int moba_batch = moba_size/12;
    MobaEx = new int[moba_batch*4];
    for(int i=8; i<moba_size; i+=12)
        MobaEx[k++] = MOBA[i];
    int moba_size_grp = moba_batch*4+4;
    fwrite(&moba_size_grp,4,1,output);
    fwrite(&moba_batch,4,1,output);
    fwrite(MobaEx,4,k,output);
    delete [] MobaEx;
    //-------INDX------------------------------------
    if (preciseVectorData)
    {
        uint32 nIdexes = nTriangles * 3;

        if(fwrite("INDX",4, 1, output) != 1)
        {
            printf("Error while writing file nbraches ID");
            exit(0);
        }
        int wsize = sizeof(uint32) + sizeof(unsigned short) * nIdexes;
        if(fwrite(&wsize, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file wsize");
            // no need to exit?
        }
        if(fwrite(&nIdexes, sizeof(uint32), 1, output) != 1)
        {
            printf("Error while writing file nIndexes");
            exit(0);
        }
        if(nIdexes >0)
        {
            if(fwrite(MOVI, sizeof(unsigned short), nIdexes, output) != nIdexes)
            {
                printf("Error while writing file indexarray");
                exit(0);
            }
        }

        if(fwrite("VERT",4, 1, output) != 1)
        {
            printf("Error while writing file nbraches ID");
            exit(0);
        }
        wsize = sizeof(int) + sizeof(float) * 3 * nVertices;
        if(fwrite(&wsize, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file wsize");
            // no need to exit?
        }
        if(fwrite(&nVertices, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file nVertices");
            exit(0);
        }
        if(nVertices >0)
        {
            if(fwrite(MOVT, sizeof(float)*3, nVertices, output) != nVertices)
            {
                printf("Error while writing file vectors");
                exit(0);
            }
        }

        nColTriangles = nTriangles;
    }
    else
    {
        //-------MOPY--------
        MoviEx = new uint16[nTriangles*3]; // "worst case" size...
        int *IndexRenum = new int[nVertices];
        memset(IndexRenum, 0xFF, nVertices*sizeof(int));
        for (int i=0; i<nTriangles; ++i)
        {
            uint16 flag = ((uint16)MOPY[2*i]) | (((uint16)MOPY[2*i+1])<<8);
            if(!isCollidable(flag))
                continue;

            // Use this triangle
            for (int j=0; j<3; ++j)
            {
                IndexRenum[MOVI[3*i + j]] = 1;
                MoviEx[3*nColTriangles + j] = MOVI[3*i + j];
            }
            ++nColTriangles;
        }

        // assign new vertex index numbers
        int nColVertices = 0;
        for (uint32 i=0; i<nVertices; ++i)
        {
            if (IndexRenum[i] == 1)
            {
                IndexRenum[i] = nColVertices;
                ++nColVertices;
            }
        }

        // translate triangle indices to new numbers
        for (int i=0; i<3*nColTriangles; ++i)
        {
            assert(MoviEx[i] < nVertices);
            MoviEx[i] = IndexRenum[MoviEx[i]];
        }

        // write triangle indices
        int INDX[] = {0x58444E49, nColTriangles*6+4, nColTriangles*3};
        fwrite(INDX,4,3,output);
        fwrite(MoviEx,2,nColTriangles*3,output);

        // write vertices
        int VERT[] = {0x54524556, nColVertices*3*static_cast<int>(sizeof(float))+4, nColVertices};// "VERT"
        int check = 3*nColVertices;
        fwrite(VERT,4,3,output);
        for (uint32 i=0; i<nVertices; ++i)
            if(IndexRenum[i] >= 0)
                check -= fwrite(MOVT+3*i, sizeof(float), 3, output);

        assert(check==0);

        delete [] MoviEx;
        delete [] IndexRenum;
    }

    //------LIQU------------------------
    if (LiquEx_size != 0)
    {
        int LIQU_h[] = {0x5551494C, static_cast<int>(sizeof(WMOLiquidHeader) + LiquEx_size) + hlq->xtiles*hlq->ytiles};// "LIQU"
        fwrite(LIQU_h, 4, 2, output);

        uint32 liquidEntry;
        if (rootWMO->liquidType & 4)
            liquidEntry = liquidType;
        else if (liquidType == 15)
            liquidEntry = 0;
        else liquidEntry = liquidType + 1;

        if (!liquidEntry)
        {
            int v1; // edx@1
            int v2; // eax@1

            v1 = hlq->xtiles * hlq->ytiles, v2 = 0;
            if (v1 > 0)
            {
                while ((LiquBytes[v2] & 0xF) == 15)
                {
                    ++v2;
                    if (v2 >= v1)
                        break;
                }

                if (v2 < v1 && (LiquBytes[v2] & 0xF) != 15)
                    liquidEntry = (LiquBytes[v2] & 0xF) + 1;
            }
        }

        if (liquidEntry && liquidEntry < 21)
        {
            switch (((uint8)liquidEntry - 1) & 3)
            {
            case 0: liquidEntry = ((mogpFlags & 0x80000) != 0) + 13; break;
            case 1: liquidEntry = 14; break;
            case 2: liquidEntry = 19; break;
            case 3: liquidEntry = 20; break;
            default: break;
            }
        }

        switch(liquidEntry)
        {
        case 13: hlq->type = MAP_LIQUID_TYPE_WATER; break;
        case 15: case 19: hlq->type = MAP_LIQUID_TYPE_MAGMA; break;
        case 20: hlq->type = MAP_LIQUID_TYPE_SLIME; break;
        case 14: hlq->type = MAP_LIQUID_TYPE_OCEAN;
            if(false) hlq->type = MAP_LIQUID_TYPE_DARK_WATER;
            break;
        }

        /* std::ofstream llog("Buildings/liquid.log", ios_base::out | ios_base::app);
        llog << filename;
        llog << ":\nliquidEntry: " << liquidEntry << " type: " << hlq->type << " (root:" << rootWMO->liquidType << " group:" << liquidType << ")\n";
        llog.close(); */

        fwrite(hlq, sizeof(WMOLiquidHeader), 1, output);
        // only need height values, the other values are unknown anyway
        for (uint32 i = 0; i<LiquEx_size/sizeof(WMOLiquidVert); ++i)
            fwrite(&LiquEx[i].height, sizeof(float), 1, output);
        // todo: compress to bit field
        fwrite(LiquBytes, 1, hlq->xtiles*hlq->ytiles, output);
    }

    return nColTriangles;
}

WMOGroup::~WMOGroup()
{
    delete [] MOPY;
    delete [] MOVI;
    delete [] MOVT;
    delete [] MOBA;
    delete hlq;
    delete [] LiquEx;
    delete [] LiquBytes;
}

WMOInstance::WMOInstance(MPQFile& f, char const* WmoInstName, uint32 mapID, uint32 tileX, uint32 tileY)
    : currx(0), curry(0), wmo(NULL), doodadset(0), indx(0), id(0), d2(0), d3(0)
{
    float ff[3];
    f.read(&id, 4);
    f.read(ff,12);
    G3D::Vector3 pos = G3D::Vector3(ff[2],ff[0],ff[1]);
    f.read(ff,12);
    G3D::Vector3 rot = G3D::Vector3(ff[0],ff[1],ff[2]);
    f.read(ff,12);
    G3D::Vector3 pos2 = G3D::Vector3(ff[2],ff[0],ff[1]);
    f.read(ff,12);
    G3D::Vector3 pos3 = G3D::Vector3(ff[2],ff[0],ff[1]);
    f.read(&d2,4);

    uint16 trash,adtId;
    f.read(&adtId,2);
    f.read(&trash,2);
    uint32 packedTile = VMAP::packTileID(tileX, tileY);
    if(tileModelSpawnSets[packedTile].find(id) == tileModelSpawnSets[packedTile].end())
        tileModelSpawnSets[packedTile].insert(id);
    if(modelSpawns.find(id) != modelSpawns.end())
        return;

    //-----------add_in _dir_file----------------

    char tempname[512];
    sprintf(tempname, "%s/%s", szWorkDirWmo, WmoInstName);
    FILE *input = fopen(tempname, "r+b");
    if(!input)
    {
        printf("WMOInstance::WMOInstance: couldn't open %s\n", tempname);
        return;
    }

    fseek(input, 8, SEEK_SET); // get the correct no of vertices
    int nVertices;
    int count = fread(&nVertices, sizeof (int), 1, input);
    fclose(input);

    if (count != 1 || nVertices == 0)
        return;

    if(pos.x == 0 && pos.y == 0)
    {
        pos.x = 533.33333f*32;
        pos.y = 533.33333f*32;
    }

    VMAP::ModelSpawn spawn;
    spawn.mapId = mapID;
    spawn.packedTile = packedTile;
    spawn.flags = MOD_HAS_BOUND | (tileX == 65 && tileY == 65 ? MOD_WORLDSPAWN : 0);
    spawn.adtId = adtId;
    spawn.ID = id;
    spawn.iPos = pos;
    spawn.iRot = rot;
    spawn.iScale = 1.0f;
    spawn.iBound = G3D::AABox(pos2, pos3);
    spawn.name = WmoInstName;
    modelSpawns.insert(std::make_pair(id, spawn));
}
