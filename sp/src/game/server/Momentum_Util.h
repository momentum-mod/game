#include "cbase.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "stdstring.h"


// Used for mounting games and re-writing the gameinfo.txt file
bool ModifyGameInfo(KeyValues* gameinfoKV, const char* appID, const char* path, bool remove)
{
	KeyValues* pOldSearchPathsKV = gameinfoKV->FindKey("FileSystem/SearchPaths");

	// Each appID requires a very specific set of VPKs to mount.  This KeyValues will keep track of these.
	KeyValues* vpk_files = new KeyValues("vpk_files");
	
	// CS:S 240
	vpk_files->SetString("240/cstrike_pak_dir", "cstrike_pak_dir");

	// TF2 440
	vpk_files->SetString("440/tf2_misc_dir", "tf2_misc_dir");
	vpk_files->SetString("440/tf2_sound_misc_dir", "tf2_sound_misc_dir");
	vpk_files->SetString("440/tf2_sound_vo_english_dir", "tf2_sound_vo_english_dir");
	vpk_files->SetString("440/tf2_textures_dir", "tf2_textures_dir");

	// Portal 400
	vpk_files->SetString("400/portal_pak_dir", "portal_pak_dir");

	// HL2: DM 320
	vpk_files->SetString("320/hl2mp_pak_dir", "hl2mp_pak_dir");

	// HL2: Episode 2 420
	vpk_files->SetString("420/ep2_pak_dir", "ep2_pak_dir");

	// HL2: Episode 1 380
	vpk_files->SetString("380/ep1_pak_dir", "ep1_pak_dir");

	// Half-Life 2 220
	vpk_files->SetString("220/hl2_misc_dir", "hl2_misc_dir");
	vpk_files->SetString("220/hl2_pak_dir", "hl2_pak_dir");
	vpk_files->SetString("220/hl2_sound_misc_dir", "hl2_sound_misc_dir");
	vpk_files->SetString("220/hl2_voice_vo_english_dir", "hl2_voice_vo_english_dir");
	vpk_files->SetString("220/hl2_textures_dir", "hl2_textures_dir");

	// Half-Life 2: Lost Coast 340
	vpk_files->SetString("340/lostcoast_pak_dir", "lostcoast_pak_dir");

	// end of vpk_files keyvalues

	// We are going to create a new SearchPaths key, then replace the existing one with the new one.
	KeyValues* pSearchPathsKV = new KeyValues("SearchPaths");

	int state = 0;
	for (KeyValues *sub = pOldSearchPathsKV->GetFirstValue(); sub; sub = sub->GetNextValue())
	{
		if (state == 0)
		{
			// 0 = looking for "endcategory" "custom"
			std::string targetPath = path;
			targetPath += "/custom/*";

			if (!Q_strcmp(sub->GetName(), "game") && !Q_strcmp(sub->GetString(), targetPath.c_str()))
			{
				// this is the the target path

				if (!remove)
				{
					// add this entry
					KeyValues* kv = pSearchPathsKV->CreateNewKey();
					kv->SetName(sub->GetName());
					kv->SetString("", sub->GetString());
				}

				// advance the state
				state++;
			}
			else if (!Q_strcmp(sub->GetName(), "endcategory") && !Q_strcmp(sub->GetString(), "custom"))
			{
				// we have reached the end of the category

				if (!remove)
				{
					// add the new entry
					KeyValues* kv = pSearchPathsKV->CreateNewKey();
					kv->SetName("game");
					kv->SetString("", targetPath.c_str());
				}

				// add this entry
				KeyValues* kv = pSearchPathsKV->CreateNewKey();
				kv->SetName(sub->GetName());
				kv->SetString("", sub->GetString());

				// advance the state
				state++;
			}
			else
			{
				// default

				// add this entry
				KeyValues* kv = pSearchPathsKV->CreateNewKey();
				kv->SetName(sub->GetName());
				kv->SetString("", sub->GetString());

				// do NOT advance the state
			}
		}
		else if (state == 1)
		{
			// 1 = looking for "endcategory" "additionalvpk"

			// Need to locate every VPK that this AppID requires
			KeyValues* app_vpk_files = vpk_files->FindKey(appID);

			if (app_vpk_files)
			{
				bool bSkip = false;
				for (KeyValues *vpk = app_vpk_files->GetFirstValue(); vpk; vpk = vpk->GetNextValue())
				{
					std::string targetPath = path;
					targetPath += "/";
					targetPath += vpk->GetString();
					targetPath += ".vpk";

					if (!Q_strcmp(sub->GetName(), "game") && !Q_strcmp(sub->GetString(), targetPath.c_str()))
					{
						// this is the the target path
						if (!remove)
						{
							// add this entry
							KeyValues* kv = pSearchPathsKV->CreateNewKey();
							kv->SetName(sub->GetName());
							kv->SetString("", sub->GetString());
						}

						// remove this from the list of needed VPKs
						vpk->SetName("");
						vpk->SetString("", "");

						bSkip = true;
						break;
					}
				}

				if (!Q_strcmp(sub->GetName(), "endcategory") && !Q_strcmp(sub->GetString(), "additionalvpk"))
				{
					// we have reached the end of the category

					if (!remove)
					{
						// add the new entries
						for (KeyValues *vpk = app_vpk_files->GetFirstValue(); vpk; vpk = vpk->GetNextValue())
						{
							if (!Q_strcmp(vpk->GetName(), ""))
								continue;

							std::string targetPath = path;
							targetPath += "/";
							targetPath += vpk->GetString();
							targetPath += ".vpk";

							KeyValues* kv = pSearchPathsKV->CreateNewKey();
							kv->SetName("game");
							kv->SetString("", targetPath.c_str());

							// remove this from the list of needed VPKs as well, for consistency's sake
							vpk->SetName("");
							vpk->SetString("", "");
						}
					}

					// add this entry
					KeyValues* kv = pSearchPathsKV->CreateNewKey();
					kv->SetName(sub->GetName());
					kv->SetString("", sub->GetString());

					// advance the state
					state++;
				}
				else if (!bSkip)
				{
					// default

					// add this entry
					KeyValues* kv = pSearchPathsKV->CreateNewKey();
					kv->SetName(sub->GetName());
					kv->SetString("", sub->GetString());

					// do NOT advance the state
				}
			}
		}
		else if (state == 2)
		{
			// 2 = looking for "endcategory" "additionalloose"

			if (!Q_strcmp(sub->GetName(), "game") && !Q_strcmp(sub->GetString(), path))
			{
				// this is the the target path

				if (!remove)
				{
					// add this entry
					KeyValues* kv = pSearchPathsKV->CreateNewKey();
					kv->SetName(sub->GetName());
					kv->SetString("", sub->GetString());
				}

				// advance the state
				state++;
			}
			else if (!Q_strcmp(sub->GetName(), "endcategory") && !Q_strcmp(sub->GetString(), "additionalloose"))
			{
				// we have reached the end of the category

				if (!remove)
				{
					// add the new entry
					KeyValues* kv = pSearchPathsKV->CreateNewKey();
					kv->SetName("game");
					kv->SetString("", path);
				}

				// add this entry
				KeyValues* kv = pSearchPathsKV->CreateNewKey();
				kv->SetName(sub->GetName());
				kv->SetString("", sub->GetString());

				// advance the state
				state++;
			}
			else
			{
				// default

				// add this entry
				KeyValues* kv = pSearchPathsKV->CreateNewKey();
				kv->SetName(sub->GetName());
				kv->SetString("", sub->GetString());

				// do NOT advance the state
			}
		}
		else if (state == 3)
		{
			// 3 = looking for "endcategory" "download"

			std::string targetPath = path;
			targetPath += "/download";

			if (!Q_strcmp(sub->GetName(), "game") && !Q_strcmp(sub->GetString(), targetPath.c_str()))
			{
				// this is the the target path

				if (!remove)
				{
					// add this entry
					KeyValues* kv = pSearchPathsKV->CreateNewKey();
					kv->SetName(sub->GetName());
					kv->SetString("", sub->GetString());
				}

				// advance the state
				state++;
			}
			else if (!Q_strcmp(sub->GetName(), "endcategory") && !Q_strcmp(sub->GetString(), "download"))
			{
				// we have reached the end of the category

				if (!remove)
				{
					// add the new entry
					KeyValues* kv = pSearchPathsKV->CreateNewKey();
					kv->SetName("game");
					kv->SetString("", targetPath.c_str());
				}

				// add this entry
				KeyValues* kv = pSearchPathsKV->CreateNewKey();
				kv->SetName(sub->GetName());
				kv->SetString("", sub->GetString());

				// advance the state
				state++;
			}
			else
			{
				// default

				// add this entry
				KeyValues* kv = pSearchPathsKV->CreateNewKey();
				kv->SetName(sub->GetName());
				kv->SetString("", sub->GetString());

				// do NOT advance the state
			}
		}
		else if (state == 4)
		{
			// 4 = trailing entry, just copy it in

			// add this entry
			KeyValues* kv = pSearchPathsKV->CreateNewKey();
			kv->SetName(sub->GetName());
			kv->SetString("", sub->GetString());

			// do NOT advance the state
		}
	}

	// Now determine if it all succeeded (thus far)
	if (state < 3)
	{
		Msg("GAMEINFO_ERROR: Failed one or more operations when modifying SearchPaths.\n");

		vpk_files->deleteThis();
		return false;
	}

	// Now replace the old SearchPaths with the new one
	pOldSearchPathsKV->Clear();
	pOldSearchPathsKV->SetName("");

	gameinfoKV->AddSubKey(pSearchPathsKV);

	vpk_files->deleteThis();
	return true;
}