#include "triggers.h"

const char *stofloat(float flValue)
{
    char buf[32];
    sprintf(buf, "%f", flValue);
    return buf;
}

const char *stoint(int iValue)
{
    char buf[32];
    sprintf(buf, "%i", iValue);
    return buf;
}

const char *stobool(bool bValue)
{
    char buf[32];
    sprintf(buf, "%i", bValue);
    return buf;
}

const char *stoVector(Vector Value)
{
    char buf[128];
    sprintf(buf, "%f %f %f", Value.x, Value.y, Value.z);
    return buf;
}

const char *stoQAngle(QAngle Value)
{
    char buf[128];
    sprintf(buf, "%f %f %f", Value.x, Value.y, Value.z);
    return buf;
}

int main(int nbargs, char **args)
{
    if (nbargs < 1)
    {
        Msg("Drag all your .bsp files on the program, they will be automatically converted into .zon files.\n");
        return 0;
    }

// Determine which filesystem to use.
#if defined(_WIN32)
    char *szFsModule = "filesystem_stdio.dll";
#elif defined(OSX)
    char *szFsModule = "filesystem_stdio.dylib";
#elif defined(LINUX)
    char *szFsModule = "filesystem_stdio.so";
#else
#error
#endif

    CSysModule *pFileSystemModule = Sys_LoadModule(szFsModule);
    Assert(pFileSystemModule);

    CreateInterfaceFn fileSystemFactory = Sys_GetFactory(pFileSystemModule);

    g_pFileSystem = (IFileSystem *)fileSystemFactory(FILESYSTEM_INTERFACE_VERSION, nullptr);

    if (g_pFileSystem == nullptr)
    {
        Msg("Couldn't get filesystem interface\n");
        return 0;
    }

    CUtlVector<entity_t *> vecTriggerEntities(0);

    for (int i = 1; i < nbargs; i++)
    {
        const char *pFilePath = args[i];

        // If it's a bsp file, we convert triggers into zon file.
        if (CUtlString(pFilePath).GetExtension() == CUtlString("bsp"))
        {
            // Unload and close the last bsp file for the next iteration.
            UnloadBSPFile();
            CloseBSPFile();

            // Load bsp file.
            LoadBSPFile(pFilePath);

            // Check also if the signature to be sure it's a bsp file.
            if (g_pBSPHeader == nullptr || g_pBSPHeader->ident != IDBSPHEADER)
            {
                Msg("(.bsp -> .zon)%s was not a bsp file!\n", pFilePath);
                continue;
            }

            // Parse all entities inside the bsp.
            ParseEntities();

            // Iterate through all entities and get all triggers wanted.
            for (int iEnt = 0; iEnt < num_entities; iEnt++)
            {
                entity_t *pEnt = &entities[iEnt];

                // Iterate through all pairs to get keyvalues.
                for (auto pEp = pEnt->epairs; pEp; pEp = pEp->next)
                {
                    // classname string contains the entity type (trigger_momentum_timer_start, etc...).
                    if (!strcmp(pEp->key, "classname"))
                    {
                        for (auto numtrigger = 0; numtrigger != triggers_max; numtrigger++)
                        {
                            char *TriggerName = m_TriggersName[numtrigger];

                            if (!strcmp(pEp->value, TriggerName))
                            {
                                // Gotcha.
                                vecTriggerEntities.AddToHead(pEnt);
                            }
                        }
                    }
                }
            }

            // Now we have all our triggers, we can generate the zon file.
            CBSPTriggers BSPTriggers;

            // Iterate through all triggers found.
            for (auto i = 0; i != vecTriggerEntities.Size(); i++)
            {
                auto triggerentity = vecTriggerEntities.Element(i);

#ifdef _DEBUG
                Msg("------- entity %p -------\n", triggerentity);

                for (auto pEp = triggerentity->epairs; pEp; pEp = pEp->next)
                {
                    Msg("%s = %s\n", pEp->key, pEp->value);
                }
#endif

                // Get the trigger type from its class name. (trigger_momentum_timer_start, etc...)
                int triggertype = GetTriggerType(ValueForKey(triggerentity, "classname"));

                // Duho, check if forgot one trigger inside the GetTriggerType, but it shouldn't happen.
                if (triggertype == -1)
                    continue;

                CBaseMomentumTrigger BaseTrigger;

                // Get the trigger origin's
                GetVectorForKey(triggerentity, "origin", BaseTrigger.m_vecPos);

                // Models contains informations about the collision, so mins, and maxs, face numbers, etc...
                auto ModelNumber = IntForKey(triggerentity, "model");

                // Get our wanted model.
                auto Model = &dmodels[ModelNumber];

                BaseTrigger.m_vecMins = Model->mins;
                BaseTrigger.m_vecMaxs = Model->maxs;

                // Get the spawnflags of triggers. (used for LimitBhopSpeed inside the start zone, etc...)
                BaseTrigger.m_spawnflags = IntForKey(triggerentity, "spawnflags");

                // This needs to be updated in case mapzones.cpp changed in server project!
                switch (triggertype)
                {
                // Start zone
                case trigger_momentum_timer_start:
                {
                    CTriggerMomentumTimerStart Trigger;
                    Trigger.m_bhopleavespeed = FloatForKey(triggerentity, "bhopleavespeed");
                    GetAnglesForKey(triggerentity, "lookangles", Trigger.m_lookangles);
                    Trigger.m_StartOnJump = IntForKey(triggerentity, "StartOnJump") != 0;
                    Trigger.m_LimitSpeedType = IntForKey(triggerentity, "LimitSpeedType");
                    Trigger.m_ZoneNumber = IntForKey(triggerentity, "ZoneNumber");
                    Trigger.Base = BaseTrigger;
                    BSPTriggers.m_TriggersMomentumTimerStart.AddToHead(Trigger);
                    break;
                }
                // End zone
                case trigger_momentum_timer_stop:
                {
                    CTriggerMomentumTimerStop Trigger;
                    Trigger.m_ZoneNumber = IntForKey(triggerentity, "ZoneNumber");
                    Trigger.Base = BaseTrigger;
                    BSPTriggers.m_TriggersMomentumTimerStop.AddToHead(Trigger);
                    break;
                }
                // One hop
                case trigger_momentum_onehop:
                {
                    CTriggerMomentumOneHop Trigger;
                    Trigger.m_hold = FloatForKey(triggerentity, "hold");
                    Trigger.m_resetang = IntForKey(triggerentity, "resetang") != 0;
                    Trigger.m_stop = FloatForKey(triggerentity, "stop") != 0;
                    Trigger.m_target = castable_string_t(ValueForKey(triggerentity, "targetname"));
                    Trigger.Base = BaseTrigger;
                    BSPTriggers.m_TriggersMomentumOneHop.AddToHead(Trigger);
                    break;
                }
                // Reset one hop
                case trigger_momentum_resetonehop:
                {
                    CTriggerMomentumResetOneHop Trigger;
                    Trigger.Base = BaseTrigger;
                    BSPTriggers.m_TriggersMomentumResetOneHop.AddToHead(Trigger);
                    break;
                }
                // Checkpoint
                case trigger_momentum_checkpoint:
                {
                    CTriggerMomentumCheckPoint Trigger;
                    Trigger.m_checkpoint = IntForKey(triggerentity, "checkpoint");
                    Trigger.Base = BaseTrigger;
                    BSPTriggers.m_TriggersMomentumCheckPoint.AddToHead(Trigger);
                    break;
                }
                // Teleport checkpoint
                case trigger_momentum_teleport_checkpoint:
                {
                    CTriggerMomentumTeleportCheckPoint Trigger;
                    Trigger.m_resetang = IntForKey(triggerentity, "resetang") != 0;
                    Trigger.m_stop = FloatForKey(triggerentity, "stop") != 0;
                    Trigger.m_target = castable_string_t(ValueForKey(triggerentity, "targetname"));
                    Trigger.Base = BaseTrigger;
                    BSPTriggers.m_TriggersMomentumTeleportCheckPoint.AddToHead(Trigger);
                    break;
                }
                // Multihop
                case trigger_momentum_multihop:
                {
                    CTriggerMomentumMultiHop Trigger;
                    Trigger.m_hold = FloatForKey(triggerentity, "hold");
                    Trigger.m_resetang = IntForKey(triggerentity, "resetang") != 0;
                    Trigger.m_stop = FloatForKey(triggerentity, "stop") != 0;
                    Trigger.m_target = castable_string_t(ValueForKey(triggerentity, "targetname"));
                    Trigger.Base = BaseTrigger;
                    BSPTriggers.m_TriggersMomentumMultiHop.AddToHead(Trigger);
                    break;
                }
                // Stage
                case trigger_momentum_timer_stage:
                {
                    CTriggerMomentumTimerStage Trigger;
                    Trigger.m_stage = IntForKey(triggerentity, "stage");
                    Trigger.Base = BaseTrigger;
                    BSPTriggers.m_TriggersMomentumTimerStage.AddToHead(Trigger);
                    break;
                }
                // ??? Seems like it missed something, but it shouldn't happen.
                default:
                    break;
                }
            }

            // Get our new file path for .zon file.
            CUtlString sFilePath(pFilePath);
            CUtlString sPathToWrite = sFilePath.StripFilename();
            CUtlString sMapName = sFilePath.GetBaseFilename();
            sPathToWrite += "\\";
            sPathToWrite += sMapName + ".zon";

            // Create keyvalues for our new zone file.
            KeyValues *ZoneKV = new KeyValues(sMapName.Get());

            // This needs to be updated in case mapzones.cpp changed in server project!

            // Iterate through all triggers elements and add the subkeyvalue with corresponding informations.

            // ProcessBaseTriggerSubKey include everything releated to positions, angles and collisions (mins & maxs).

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumTimerStart.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumTimerStart.Element(i);

                KeyValues *SubZoneKey = new KeyValues(m_ZoneFileTriggersName[trigger_momentum_timer_start]);

                SubZoneKey->SetFloat("bhopleavespeed", Element.m_bhopleavespeed);
                SubZoneKey->SetBool("limitingspeed", (Element.Base.m_spawnflags & SF_LIMIT_LEAVE_SPEED) ? true : false);
                SubZoneKey->SetBool("StartOnJump", Element.m_StartOnJump);
                SubZoneKey->SetInt("LimitSpeedType", Element.m_LimitSpeedType);
                SubZoneKey->SetInt("ZoneNumber", Element.m_ZoneNumber);
                SubZoneKey->SetFloat("yaw", Element.m_lookangles[YAW]);
                ProcessBaseTriggerSubKey(SubZoneKey, &Element.Base);
                ZoneKV->AddSubKey(SubZoneKey);
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumTimerStop.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumTimerStop.Element(i);

                KeyValues *SubZoneKey = new KeyValues(m_ZoneFileTriggersName[trigger_momentum_timer_stop]);
                SubZoneKey->SetInt("ZoneNumber", Element.m_ZoneNumber);
                ProcessBaseTriggerSubKey(SubZoneKey, &Element.Base);
                ZoneKV->AddSubKey(SubZoneKey);
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumOneHop.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumOneHop.Element(i);

                KeyValues *SubZoneKey = new KeyValues(m_ZoneFileTriggersName[trigger_momentum_onehop]);
                SubZoneKey->SetBool("stop", Element.m_stop);
                SubZoneKey->SetBool("resetang", Element.m_resetang);
                SubZoneKey->SetFloat("hold", Element.m_hold);
                SubZoneKey->SetString("destinationname", Element.m_target.ToCStr());
                ProcessBaseTriggerSubKey(SubZoneKey, &Element.Base);
                ZoneKV->AddSubKey(SubZoneKey);
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumResetOneHop.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumResetOneHop.Element(i);

                KeyValues *SubZoneKey = new KeyValues(m_ZoneFileTriggersName[trigger_momentum_resetonehop]);
                ProcessBaseTriggerSubKey(SubZoneKey, &Element.Base);
                ZoneKV->AddSubKey(SubZoneKey);
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumMultiHop.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumMultiHop.Element(i);

                KeyValues *SubZoneKey = new KeyValues(m_ZoneFileTriggersName[trigger_momentum_multihop]);
                SubZoneKey->SetBool("stop", Element.m_stop);
                SubZoneKey->SetBool("resetang", Element.m_resetang);
                SubZoneKey->SetFloat("hold", Element.m_hold);
                SubZoneKey->SetString("destinationname", Element.m_target.ToCStr());
                ProcessBaseTriggerSubKey(SubZoneKey, &Element.Base);
                ZoneKV->AddSubKey(SubZoneKey);
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumCheckPoint.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumCheckPoint.Element(i);

                KeyValues *SubZoneKey = new KeyValues(m_ZoneFileTriggersName[trigger_momentum_checkpoint]);
                SubZoneKey->SetInt("number", Element.m_checkpoint);
                ProcessBaseTriggerSubKey(SubZoneKey, &Element.Base);
                ZoneKV->AddSubKey(SubZoneKey);
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumTeleportCheckPoint.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumTeleportCheckPoint.Element(i);

                KeyValues *SubZoneKey = new KeyValues(m_ZoneFileTriggersName[trigger_momentum_teleport_checkpoint]);
                SubZoneKey->SetBool("stop", Element.m_stop);
                SubZoneKey->SetBool("resetang", Element.m_resetang);
                SubZoneKey->SetString("destinationname", Element.m_target.ToCStr());
                ProcessBaseTriggerSubKey(SubZoneKey, &Element.Base);
                ZoneKV->AddSubKey(SubZoneKey);
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumTimerStage.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumTimerStage.Element(i);

                KeyValues *SubZoneKey = new KeyValues(m_ZoneFileTriggersName[trigger_momentum_timer_stage]);
                SubZoneKey->SetInt("number", Element.m_stage);
                ProcessBaseTriggerSubKey(SubZoneKey, &Element.Base);
                ZoneKV->AddSubKey(SubZoneKey);
            }

            // Save our keyvalues in the new zone file.
            if (!ZoneKV->SaveToFile(g_pFileSystem, sPathToWrite.Get()))
            {
                Msg("Couldn't write the new zone for %s\n", sMapName.Get());
            }

            // delete.
            ZoneKV->deleteThis();
        }
        // If it's a zon file, we read and write the bsp with assuming that it's on the same path.
        else if (CUtlString(pFilePath).GetExtension() == CUtlString("zon"))
        {
            CUtlString sFilePath(pFilePath);
            CUtlString sPathToWrite = sFilePath.StripFilename();
            CUtlString sMapName = sFilePath.GetBaseFilename();
            sPathToWrite += "\\";
            sPathToWrite += sMapName + ".bsp";

            KeyValues *ZoneKV = new KeyValues(sMapName.Get());

            // Read keyvalues.
            ZoneKV->LoadFromFile(g_pFileSystem, pFilePath);

            if (ZoneKV->IsEmpty())
            {
                Msg("Couldn't read %s zone file!\n", pFilePath);
                continue;
            }

            CBSPTriggers BSPTriggers;

            // Load all zones in our vectors of triggers.
            for (auto SubZoneKey = ZoneKV->GetFirstSubKey(); SubZoneKey; SubZoneKey = SubZoneKey->GetNextKey())
            {
                auto zonename = SubZoneKey->GetName();
                auto triggertype = GetZoneFileTriggerType(zonename);

                CBaseMomentumTrigger BaseTrigger;
                ProcessSubKeyBaseTrigger(SubZoneKey, &BaseTrigger);

                // This needs to be updated in case mapzones.cpp changed in server project!
                switch (triggertype)
                {
                // Start zone
                case trigger_momentum_timer_start:
                {
                    CTriggerMomentumTimerStart Trigger;
                    Trigger.Base = BaseTrigger;
                    Trigger.m_bhopleavespeed = SubZoneKey->GetFloat("bhopleavespeed");
                    Trigger.Base.m_spawnflags = SubZoneKey->GetBool("limitingspeed") ? SF_LIMIT_LEAVE_SPEED : 0;
                    Trigger.m_StartOnJump = SubZoneKey->GetBool("StartOnJump");
                    Trigger.m_LimitSpeedType = SubZoneKey->GetInt("LimitSpeedType");
                    Trigger.m_ZoneNumber = SubZoneKey->GetInt("ZoneNumber");
                    Trigger.m_lookangles[YAW] = SubZoneKey->GetFloat("yaw");
                    BSPTriggers.m_TriggersMomentumTimerStart.AddToHead(Trigger);
                    break;
                }
                // End zone
                case trigger_momentum_timer_stop:
                {
                    CTriggerMomentumTimerStop Trigger;
                    Trigger.Base = BaseTrigger;
                    Trigger.m_ZoneNumber = SubZoneKey->GetInt("ZoneNumber");
                    BSPTriggers.m_TriggersMomentumTimerStop.AddToHead(Trigger);
                    break;
                }
                // One hop
                case trigger_momentum_onehop:
                {
                    CTriggerMomentumOneHop Trigger;
                    Trigger.Base = BaseTrigger;
                    Trigger.m_stop = SubZoneKey->GetBool("stop");
                    Trigger.m_resetang = SubZoneKey->GetBool("resetang");
                    Trigger.m_hold = SubZoneKey->GetFloat("hold");
                    Trigger.m_target = MAKE_STRING(SubZoneKey->GetString("destinationname"));
                    BSPTriggers.m_TriggersMomentumOneHop.AddToHead(Trigger);
                    break;
                }
                // Reset one hop
                case trigger_momentum_resetonehop:
                {
                    CTriggerMomentumResetOneHop Trigger;
                    Trigger.Base = BaseTrigger;
                    BSPTriggers.m_TriggersMomentumResetOneHop.AddToHead(Trigger);
                    break;
                }
                // Checkpoint
                case trigger_momentum_checkpoint:
                {
                    CTriggerMomentumCheckPoint Trigger;
                    Trigger.Base = BaseTrigger;
                    Trigger.m_checkpoint = SubZoneKey->GetInt("number");
                    BSPTriggers.m_TriggersMomentumCheckPoint.AddToHead(Trigger);
                    break;
                }
                // Teleport checkpoint
                case trigger_momentum_teleport_checkpoint:
                {
                    CTriggerMomentumTeleportCheckPoint Trigger;
                    Trigger.Base = BaseTrigger;
                    Trigger.m_stop = SubZoneKey->GetBool("stop");
                    Trigger.m_resetang = SubZoneKey->GetBool("resetang");
                    Trigger.m_target = MAKE_STRING(SubZoneKey->GetString("destinationname"));
                    break;
                }
                // Multihop
                case trigger_momentum_multihop:
                {
                    CTriggerMomentumMultiHop Trigger;
                    Trigger.Base = BaseTrigger;
                    Trigger.m_stop = SubZoneKey->GetBool("stop");
                    Trigger.m_resetang = SubZoneKey->GetBool("resetang");
                    Trigger.m_hold = SubZoneKey->GetFloat("hold");
                    Trigger.m_target = MAKE_STRING(SubZoneKey->GetString("destinationname"));
                    BSPTriggers.m_TriggersMomentumMultiHop.AddToHead(Trigger);
                    break;
                }
                // Stage
                case trigger_momentum_timer_stage:
                {
                    CTriggerMomentumTimerStage Trigger;
                    Trigger.Base = BaseTrigger;
                    Trigger.m_stage = SubZoneKey->GetInt("number");
                    BSPTriggers.m_TriggersMomentumTimerStage.AddToHead(Trigger);
                    break;
                }
                // ??? Seems like it missed something, but it shouldn't happen.
                default:
                    break;
                }
            }

            ZoneKV->deleteThis();

            // Unload and close the last bsp file for the next iteration.
            UnloadBSPFile();
            CloseBSPFile();

            // Load bsp file.
            LoadBSPFile(sPathToWrite.Get());

            // Check also if the signature to be sure it's a bsp file.
            if (g_pBSPHeader == nullptr || g_pBSPHeader->ident != IDBSPHEADER)
            {
                Msg("(.zon -> .bsp) %s was not a bsp file!\n", pFilePath);
                continue;
            }

            // Convert the vectors of triggers into .bsp files.
            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumTimerStart.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumTimerStart.Element(i);
                auto pEnt = &entities[num_entities];
                SetKeyValue(pEnt, "bhopleavespeed", stofloat(Element.m_bhopleavespeed));
                SetKeyValue(pEnt, "lookangles", stoQAngle(Element.m_lookangles));
                SetKeyValue(pEnt, "StartOnJump", stobool(Element.m_StartOnJump));
                SetKeyValue(pEnt, "ZoneNumber", stobool(Element.m_ZoneNumber));
                num_entities++;
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumTimerStop.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumTimerStop.Element(i);
                auto pEnt = &entities[num_entities];
                num_entities++;
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumOneHop.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumOneHop.Element(i);
                auto pEnt = &entities[num_entities];
                num_entities++;
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumResetOneHop.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumResetOneHop.Element(i);
                auto pEnt = &entities[num_entities];
                num_entities++;
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumMultiHop.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumMultiHop.Element(i);
                auto pEnt = &entities[num_entities];
                num_entities++;
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumCheckPoint.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumCheckPoint.Element(i);
                auto pEnt = &entities[num_entities];
                num_entities++;
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumTeleportCheckPoint.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumTeleportCheckPoint.Element(i);
                auto pEnt = &entities[num_entities];
                num_entities++;
            }

            for (auto i = 0; i != BSPTriggers.m_TriggersMomentumTimerStage.Size(); i++)
            {
                auto Element = BSPTriggers.m_TriggersMomentumTimerStage.Element(i);
                auto pEnt = &entities[num_entities];
                num_entities++;
            }

            UnparseEntities();
            WriteBSPFile(sPathToWrite.Get());
        }
        else
        {
            // Somehow it wasn't a bsp or a zon file when the user dragged the files.
            Msg("%s was not a valid file!\n", pFilePath);
        }
    }

    // Be sure we cleared the bsp file out.
    UnloadBSPFile();
    CloseBSPFile();

    system("pause");

    return 1;
}