/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

bool HandleInfoCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleSuicideCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleGMsCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleAnnounceCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleWAnnounceCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleKickCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleQuitCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleRestartCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleCancelCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleUptimeCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleBanAccountCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleUnbanAccountCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleMOTDCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleOnlinePlayersCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandlePlayerInfoCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleCreateAccountCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleRehashCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleBackupDBCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleSaveAllCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleWhisperCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleNameHashCommand(BaseConsole * pConsole, int argc, const char * argv[]);
