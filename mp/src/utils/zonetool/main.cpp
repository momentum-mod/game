#include "byteswap.h"
#include "detail.h"
#include "disp_vbsp.h"
#include "ivp.h"
#include "loadcmdline.h"
#include "map.h"
#include "materialsub.h"
#include "materialsystem/imaterialsystem.h"
#include "physdll.h"
#include "tier0/icommandline.h"
#include "tools_minidump.h"
#include "triggers.h"
#include "utilmatlib.h"
#include "vbsp.h"
#include "worldvertextransitionfixup.h"
#include "writebsp.h"

// parameters for conversion to vphysics
#define NO_SHRINK 0.0f
// NOTE: vphysics maintains a minimum separation radius between objects
// This radius is set to 0.25, but it's symmetric.  So shrinking potentially moveable
// brushes by 0.5 in every direction ensures that these brushes can be constructed
// touching the world, and constrained in place without collisions or friction
// UNDONE: Add a key to disable this shrinking if necessary
#define VPHYSICS_SHRINK (0.5f) // shrink BSP brushes by this much for collision
#define VPHYSICS_MERGE 0.01f   // me

extern qboolean onlyents;
extern qboolean dumpcollide;

extern ChunkFileResult_t LoadEntityCallback(CChunkFile *pFile, int nParam);
extern void ProcessModels();
extern void ProcessSubModel();
extern void ProcessWorldModel();
extern void FixupOnlyEntsOccluderEntities();
extern void EmitBrushes();
extern void EmitPhysCollision();
extern void EmitPlanes();
extern void BuildWorldPhysModel(CUtlVector<CPhysCollisionEntry *> &collisionList, float shrinkSize,
                                float mergeTolerance);
extern void ConvertModelToPhysCollide(CUtlVector<CPhysCollisionEntry *> &collisionList, int modelIndex, int contents,
                                      float shrinkSize, float mergeTolerance);
extern IPhysicsSurfaceProps *physprops;
extern void CompactTexinfos();

extern void EmitOccluderBrushes();

int main(int nbargs, char **args)
{
    // if only ents, it doesn't load brushes etc we don't want that.
    // onlyents = true;
    dumpcollide = true;
    MathLib_Init();

    if (nbargs < 1)
    {
        Msg("Drag all your .bsp files on the program, they will be automatically converted into .zon files.\n");
        return 0;
    }

    CommandLine()->CreateCmdLine(nbargs, args);
    CUtlString vproject_arg(args[0]);
    vproject_arg = vproject_arg.StripFilename();
    vproject_arg = vproject_arg.Slice(0, vproject_arg.Length() - 3);
    char buf[1024];
    sprintf(buf, "-vproject -game %smomentum", vproject_arg.Get());
    CommandLine()->CreateCmdLine(buf);

    CmdLib_InitFileSystem(args[0]);

    // Setup the logfile.
    char logFile[512];
    _snprintf(logFile, sizeof(logFile), "%s.log", source);
    SetSpewFunctionLogFile(logFile);

    PhysicsDLLPath("vphysics.dll");
    LoadSurfaceProperties();

    ThreadSetDefault();
    numthreads = 1; // multiple threads aren't helping...

    char materialPath[MAX_PATH];
    sprintf(materialPath, "%smaterials", gamedir);
    InitMaterialSystem(materialPath, CmdLib_GetFileSystemFactory());
    Msg("materialPath: %s\n", materialPath);

    if ((g_nDXLevel != 0) && (g_nDXLevel < 80))
    {
        g_BumpAll = false;
    }

    if (g_luxelScale == 1.0f)
    {
        if (g_nDXLevel == 70)
        {
            g_luxelScale = 4.0f;
        }
    }

    for (int iarg = 1; iarg < nbargs; iarg++)
    {
        const char *pFilePath = args[iarg];

        CUtlString sFilePath(pFilePath);

        // If it's a bsp file, we convert triggers into zon file.
        if (sFilePath.GetExtension() == CUtlString("bsp"))
        {
            // Get vmf file.
            CUtlString sFilePath_VMF(sFilePath.StripExtension() + ".vmf");

            // Load bsp.
            LoadBSPFile(pFilePath);
            ParseEntities();

            PrintBSPFileSizes();

            LoadMapFile(sFilePath_VMF.Get());

            g_MainMap->map_mins = dmodels[0].mins;
            g_MainMap->map_maxs = dmodels[0].maxs;

            PrintBSPFileSizes();

            int iOldModelNumber = nummodels;

            int iOldNumEntities = num_entities;

            {
                for (int i = num_entities; i < g_MainMap->num_entities; i++)
                {
                    memcpy(&entities[num_entities], &g_MainMap->entities[i], sizeof(entity_t));

                    char ModelNumber[10], swithoutModelNumber[10];
                    sprintf(ModelNumber, "*%i", iOldModelNumber);
                    sprintf(swithoutModelNumber, "%i", iOldModelNumber);
                    SetKeyValue(&entities[num_entities], "model", ModelNumber);
                    // Random
                    SetKeyValue(&entities[num_entities], "hammerid",
                                CUtlString(CUtlString("5461") + CUtlString(swithoutModelNumber)).Get());

                    iOldModelNumber++;
                    num_entities++;
                }
            }

            iOldModelNumber = nummodels;

            // num_entities += g_MainMap->num_entities;
            // EmitInitialDispInfos();
            // EmitOccluderBrushes();

            // Process our current model, we need to check if g_MainMap is used wrongly in those functions, but as
            // looked a lot of times, I don't know where could be the problem here. But it must be here I think.
            for (entity_num = iOldNumEntities; entity_num < num_entities; ++entity_num)
            {
                entity_t *pEntity = &entities[entity_num];
                if (!pEntity->numbrushes)
                    continue;

                qprintf("############### model %i ###############\n", nummodels);

                BeginModel();

                if (entity_num == 0)
                {
                    ProcessWorldModel();
                }
                else
                {
                    ProcessSubModel();
                }

                EndModel();
            }

            {
                int i, j, bnum, s, x;
                dbrush_t *db;
                mapbrush_t *b;
                dbrushside_t *cp;
                Vector normal;
                vec_t dist;
                int planenum;

                // We add only the brushes we need, otherwhise it just copy empty brushes and we don't want that.
                for (bnum = numbrushes; bnum < g_MainMap->nummapbrushes; bnum++)
                {
                    b = &g_MainMap->mapbrushes[bnum];
                    db = &dbrushes[bnum];

                    db->contents = b->contents;
                    db->firstside = numbrushsides;
                    db->numsides = b->numsides;
                    for (j = 0; j < b->numsides; j++)
                    {
                        if (numbrushsides == MAX_MAP_BRUSHSIDES)
                            Error("MAX_MAP_BRUSHSIDES");
                        cp = &dbrushsides[numbrushsides];
                        numbrushsides++;
                        cp->planenum = b->original_sides[j].planenum;
                        cp->texinfo = b->original_sides[j].texinfo;
                        if (cp->texinfo == -1)
                        {
                            cp->texinfo = g_MainMap->g_ClipTexinfo;
                        }
                        cp->bevel = b->original_sides[j].bevel;
                    }

                    // add any axis planes not contained in the brush to bevel off corners
                    for (x = 0; x < 3; x++)
                        for (s = -1; s <= 1; s += 2)
                        {
                            // add the plane
                            VectorCopy(vec3_origin, normal);
                            normal[x] = s;
                            if (s == -1)
                                dist = -b->mins[x];
                            else
                                dist = b->maxs[x];
                            planenum = g_MainMap->FindFloatPlane(normal, dist);
                            for (i = 0; i < b->numsides; i++)
                                if (b->original_sides[i].planenum == planenum)
                                    break;
                            if (i == b->numsides)
                            {
                                if (numbrushsides >= MAX_MAP_BRUSHSIDES)
                                    Error("MAX_MAP_BRUSHSIDES");

                                dbrushsides[numbrushsides].planenum = planenum;
                                dbrushsides[numbrushsides].texinfo = dbrushsides[numbrushsides - 1].texinfo;
                                numbrushsides++;
                                db->numsides++;
                            }
                        }
                }

                numbrushes = g_MainMap->nummapbrushes;
            }

            // Copy new planes, only the ones we need.
            {
                int i;
                dplane_t *dp;

                auto planestoadd = g_MainMap->nummapplanes - numplanes;
                int ioldnumplanes = numplanes;

                for (i = 0; i < planestoadd; i++)
                {
                    auto mp = &g_MainMap->mapplanes[i + ioldnumplanes];
                    dp = &dplanes[numplanes];
                    VectorCopy(mp->normal, dp->normal);
                    dp->dist = mp->dist;
                    dp->type = mp->type;
                    numplanes++;
                }
            }

            {
                // Create all collisions to our models
                // , tested and it seems to produce correctly the binary, (compared with the original's collisions data
                // with dumps to be sure its same bytes, and it is).

                CreateInterfaceFn physicsFactory = GetPhysicsFactory();
                if (physicsFactory)
                {
                    physcollision = (IPhysicsCollision *)physicsFactory(VPHYSICS_COLLISION_INTERFACE_VERSION, NULL);
                }

                if (!physcollision)
                {
                    Warning("!!! WARNING: Can't build collision data!\n");
                    return 0;
                }

                CUtlVector<CPhysCollisionEntry *> collisionList[MAX_MAP_MODELS];
                CTextBuffer *pTextBuffer[MAX_MAP_MODELS];

                int physModelCount = 0, totalSize = 0;

                int start = Plat_FloatTime();

                Msg("Building Physics collision data...\n");

                int i, j;
                for (i = iOldModelNumber; i < nummodels; i++)
                {
                    ConvertModelToPhysCollide(collisionList[i], i,
                                              MASK_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP | MASK_WATER,
                                              VPHYSICS_SHRINK, VPHYSICS_MERGE);

                    pTextBuffer[i] = NULL;
                    if (!collisionList[i].Count())
                        continue;

                    // if we've got collision models, write their script for processing in the game
                    pTextBuffer[i] = new CTextBuffer;
                    for (j = 0; j < collisionList[i].Count(); j++)
                    {
                        // dump a text file for visualization
                        if (dumpcollide)
                        {
                            collisionList[i][j]->DumpCollide(pTextBuffer[i], i, j);
                        }
                        // each model knows how to write its script
                        collisionList[i][j]->WriteToTextBuffer(pTextBuffer[i], i, j);
                        // total up the binary section's size
                        totalSize += collisionList[i][j]->GetCollisionBinarySize() + sizeof(int);
                    }

                    pTextBuffer[i]->Terminate();

                    // total lump size includes the text buffers (scripts)
                    totalSize += pTextBuffer[i]->GetSize();

                    physModelCount++;
                }

                //  add one for tail of list marker
                physModelCount++;

                // Save up our original collision data.

                // - 1 model because the last one is to tell when it's ended in models enumerations.
                auto OldCollideSize = g_PhysCollideSize - sizeof(dphysmodel_t);
                auto pOldPhysCollide = malloc(OldCollideSize);

                memcpy(pOldPhysCollide, g_pPhysCollide, OldCollideSize);

                // DWORD align the lump because AddLump assumes that it is DWORD aligned.
                byte *ptr;
                g_PhysCollideSize = totalSize + (physModelCount * sizeof(dphysmodel_t)) + OldCollideSize;
                g_pPhysCollide = (byte *)malloc((g_PhysCollideSize + 3) & ~3);
                memset(g_pPhysCollide, 0, g_PhysCollideSize);
                ptr = g_pPhysCollide;

                for (i = iOldModelNumber; i < nummodels; i++)
                {
                    if (pTextBuffer[i])
                    {
                        int j;

                        dphysmodel_t model;

                        model.modelIndex = i;
                        model.solidCount = collisionList[i].Count();
                        model.dataSize = sizeof(int) * model.solidCount;

                        for (j = 0; j < model.solidCount; j++)
                        {
                            model.dataSize += collisionList[i][j]->GetCollisionBinarySize();
                        }
                        model.keydataSize = pTextBuffer[i]->GetSize();

                        // store the header
                        memcpy(ptr, &model, sizeof(model));
                        ptr += sizeof(model);

                        for (j = 0; j < model.solidCount; j++)
                        {
                            int collideSize = collisionList[i][j]->GetCollisionBinarySize();

                            // write size
                            memcpy(ptr, &collideSize, sizeof(int));
                            ptr += sizeof(int);

                            // now write the collision model
                            collisionList[i][j]->WriteCollisionBinary(reinterpret_cast<char *>(ptr));
                            ptr += collideSize;
                        }

                        memcpy(ptr, pTextBuffer[i]->GetData(), pTextBuffer[i]->GetSize());
                        ptr += pTextBuffer[i]->GetSize();
                    }

                    delete pTextBuffer[i];
                }

                // Copy old collisions.
                memcpy(ptr, pOldPhysCollide, OldCollideSize);
                ptr += OldCollideSize;

                dphysmodel_t model;

                // Mark end of list
                model.modelIndex = -1;
                model.dataSize = -1;
                model.keydataSize = 0;
                model.solidCount = 0;
                memcpy(ptr, &model, sizeof(model));
                ptr += sizeof(model);
                Assert((ptr - g_pPhysCollide) == g_PhysCollideSize);
                Msg("done (%d) (%d bytes)\n", (int)(Plat_FloatTime() - start), g_PhysCollideSize);
                free(pOldPhysCollide);
            }

            UnparseEntities();

            // Write our new bsp file.
            CUtlString pNewFilePath(pFilePath);
            pNewFilePath = pNewFilePath.StripExtension();
            pNewFilePath += "_new.bsp";

            PrintBSPFileSizes();
            WriteBSPFile(pNewFilePath.Get());

            LoadMapFile(sFilePath_VMF.Get());

            // Some test if somehow all our models weren't written. But should never happen.
            /*LoadBSPFile(pNewFilePath.Get());

            ParseEntities();

            byte *ptr = g_pPhysCollide;
            byte *basePtr = ptr;

            dphysmodel_t physModel;

            CUtlVector<vcollide_t> vec_collides;

            // physics data is variable length.  The last physmodel is a NULL pointer
            // with modelIndex -1, dataSize -1
            do
            {
                memcpy(&physModel, ptr, sizeof(physModel));
                ptr += sizeof(physModel);

                if (physModel.dataSize > 0)
                {
                    vcollide_t collide;
                    physcollision->VCollideLoad(&collide, physModel.solidCount, (const char *)ptr,
                                                physModel.dataSize + physModel.keydataSize);
                    ptr += physModel.dataSize;
                    ptr += physModel.keydataSize;

                    vec_collides.AddToHead(collide);
                }

                // avoid infinite loop on badly formed file
                if ((int)(ptr - basePtr) > g_PhysCollideSize)
                    break;

            } while (physModel.dataSize > 0);

            vcollide_t vcollide = vec_collides.Element(vec_collides.Count()-2);
            vcollide_t oldvcollide = vec_collides.Element(vec_collides.Count() - 3);

            vcollide = vcollide;
            oldvcollide = oldvcollide;

            UnparseEntities();
            PrintBSPFileSizes();
            WriteBSPFile(pNewFilePath.Get());*/
        }
    }
}