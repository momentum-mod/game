#pragma once

// CREDITS: Valve.

// We must have our own renderer for specific brushes models, because we can't override any material with it.

#include "cbase.h"
#include "model_types.h"

struct msurface2_t;
struct worldbrushdata_t;
struct decal_t;
class CCommonHostState;
struct model_t;
struct LightShadowZBufferSample_t;
struct RayDispOutput_t;

extern CCommonHostState *host_state;

#define WORLD_DECAL_HANDLE_INVALID = 0xFFFF

// drawing surface flags
#define SURFDRAW_NOLIGHT 0x00000001        // no lightmap
#define SURFDRAW_NODE 0x00000002           // This surface is on a node
#define SURFDRAW_SKY 0x00000004            // portal to sky
#define SURFDRAW_BUMPLIGHT 0x00000008      // Has multiple lightmaps for bump-mapping
#define SURFDRAW_NODRAW 0x00000010         // don't draw this surface, not really visible
#define SURFDRAW_TRANS 0x00000020          // sort this surface from back to front
#define SURFDRAW_PLANEBACK 0x00000040      // faces away from plane of the node that stores this face
#define SURFDRAW_DYNAMIC 0x00000080        // Don't use a static buffer for this face
#define SURFDRAW_TANGENTSPACE 0x00000100   // This surface needs a tangent space
#define SURFDRAW_NOCULL 0x00000200         // Don't bother backface culling these
#define SURFDRAW_HASLIGHTSYTLES 0x00000400 // has a lightstyle other than 0
#define SURFDRAW_HAS_DISP 0x00000800       // has a dispinfo
#define SURFDRAW_ALPHATEST 0x00001000      // Is alphstested
#define SURFDRAW_NOSHADOWS 0x00002000      // No shadows baby
#define SURFDRAW_NODECALS 0x00004000       // No decals baby
#define SURFDRAW_HAS_PRIMS 0x00008000      // has a list of prims
#define SURFDRAW_WATERSURFACE 0x00010000   // This is a water surface
#define SURFDRAW_UNDERWATER 0x00020000
#define SURFDRAW_ABOVEWATER 0x00040000
#define SURFDRAW_HASDLIGHT 0x00080000      // Has some kind of dynamic light that must be checked
#define SURFDRAW_DLIGHTPASS 0x00100000     // Must be drawn in the dlight pass
#define SURFDRAW_UNUSED2 0x00200000        // unused
#define SURFDRAW_VERTCOUNT_MASK 0xFF000000 // 8 bits of vertex count
#define SURFDRAW_SORTGROUP_MASK 0x00C00000 // 2 bits of sortgroup

#define SURFDRAW_VERTCOUNT_SHIFT 24
#define SURFDRAW_SORTGROUP_SHIFT 22

#define MAX_SURFINFO_VERTS 16

#define SHADOW_ZBUF_RES 8 // 6 * 64 * 2 * 4 = 3k bytes per light

template <class T, int RES> struct CCubeMap
{
    T m_Samples[6][RES][RES];

  public:
    FORCEINLINE void GetCoords(Vector const &vecNormalizedDirection, int &nX, int &nY, int &nFace)
    {
        // find largest magnitude component
        int nLargest = 0;
        int nAxis0 = 1;
        int nAxis1 = 2;
        if (fabs(vecNormalizedDirection[1]) > fabs(vecNormalizedDirection[0]))
        {
            nLargest = 1;
            nAxis0 = 0;
            nAxis1 = 2;
        }
        if (fabs(vecNormalizedDirection[2]) > fabs(vecNormalizedDirection[nLargest]))
        {
            nLargest = 2;
            nAxis0 = 0;
            nAxis1 = 1;
        }
        float flZ = vecNormalizedDirection[nLargest];
        if (flZ < 0)
        {
            flZ = -flZ;
            nLargest += 3;
        }
        nFace = nLargest;
        flZ = 1.0 / flZ;
        nX = RemapValClamped(vecNormalizedDirection[nAxis0] * flZ, -1, 1, 0, RES - 1);
        nY = RemapValClamped(vecNormalizedDirection[nAxis1] * flZ, -1, 1, 0, RES - 1);
    }

    FORCEINLINE T &GetSample(Vector const &vecNormalizedDirection)
    {
        int nX, nY, nFace;
        GetCoords(vecNormalizedDirection, nX, nY, nFace);
        return m_Samples[nFace][nX][nY];
    }
};

typedef unsigned short WorldDecalHandle_t;

typedef dleafambientindex_t mleafambientindex_t;
typedef dleafambientlighting_t mleafambientlighting_t;

typedef CCubeMap<LightShadowZBufferSample_t, SHADOW_ZBUF_RES> lightzbuffer_t;

//-----------------------------------------------------------------------------
// Shadow decals are applied to a single surface
//-----------------------------------------------------------------------------
typedef unsigned short ShadowDecalHandle_t;

enum
{
    SHADOW_DECAL_HANDLE_INVALID = (ShadowDecalHandle_t)~0
};

//-----------------------------------------------------------------------------
// Overlay fragments
//-----------------------------------------------------------------------------
typedef unsigned short OverlayFragmentHandle_t;

enum
{
    OVERLAY_FRAGMENT_INVALID = (OverlayFragmentHandle_t)~0
};

//-----------------------------------------------------------------------------
// Handle to decals + shadows on displacements
//-----------------------------------------------------------------------------
typedef unsigned short DispDecalHandle_t;

enum
{
    DISP_DECAL_HANDLE_INVALID = (DispDecalHandle_t)~0
};

typedef unsigned short DispShadowHandle_t;

enum
{
    DISP_SHADOW_HANDLE_INVALID = (DispShadowHandle_t)~0
};

typedef void *HDISPINFOARRAY;

typedef msurface2_t *SurfaceHandle_t;

class CCommonHostState
{
  public:
    model_t *worldmodel; // cl_entitites[0].model
    worldbrushdata_t *worldbrush;
    float interval_per_tick; // Tick interval for game
};

// Used by GetIntersectingSurfaces.
class GetIntersectingSurfaces_Struct
{
  public:
    model_t *m_pModel;
    const Vector *m_pCenter;
    const byte *m_pCenterPVS; // PVS for the leaf m_pCenter is in.
    float m_Radius;
    bool m_bOnlyVisible;

    SurfInfo *m_pInfos;
    int m_nMaxInfos;
    int m_nSetInfos;
};

//-----------------------------------------------------------------------------
// Displacement interface to the engine (and WorldCraft?)
//-----------------------------------------------------------------------------
abstract_class IDispInfo
{
  public:
    virtual ~IDispInfo() {}

    // Builds a list of displacement triangles intersecting the sphere.
    virtual void GetIntersectingSurfaces(GetIntersectingSurfaces_Struct * pStruct) = 0;

    virtual void RenderWireframeInLightmapPage(int pageId) = 0;

    virtual void GetBoundingBox(Vector & bbMin, Vector & bbMax) = 0;

    // Get and set the parent surfaces.
    virtual void SetParent(SurfaceHandle_t surfID) = 0;
    virtual SurfaceHandle_t GetParent() = 0;

    // Add dynamic lights to the lightmap for this surface.
    virtual void AddDynamicLights(struct dlight_t * pLights, unsigned int lightMask) = 0;
    // Compute the mask for the lights hitting this surface.
    virtual unsigned int ComputeDynamicLightMask(struct dlight_t * pLights) = 0;

    // Add and remove decals.
    // flSize is like the radius of the decal so the decal isn't put on any disp faces it's too far away from.
    virtual DispDecalHandle_t NotifyAddDecal(decal_t * pDecal, float flSize) = 0;
    virtual void NotifyRemoveDecal(DispDecalHandle_t h) = 0;

    virtual DispShadowHandle_t AddShadowDecal(ShadowHandle_t shadowHandle) = 0;
    virtual void RemoveShadowDecal(DispShadowHandle_t handle) = 0;

    // Compute shadow fragments for a particular shadow, return the vertex + index count of all fragments
    virtual bool ComputeShadowFragments(DispShadowHandle_t h, int &vertexCount, int &indexCount) = 0;

    // Tag the surface and check if it's tagged. You can untag all the surfaces
    // with DispInfo_ClearAllTags. Note: it just uses a frame counter to track the
    // tag state so it's really really fast to call ClearAllTags (just increments
    // a variable 99.999% of the time).
    virtual bool GetTag() = 0;
    virtual void SetTag() = 0;

    // Cast a ray against this surface
    virtual bool TestRay(Ray_t const &ray, float start, float end, float &dist, Vector2D *lightmapUV,
                         Vector2D *textureUV) = 0;

    // Computes the texture + lightmap coordinate given a displacement uv
    virtual void ComputeLightmapAndTextureCoordinate(RayDispOutput_t const &uv, Vector2D *luv, Vector2D *tuv) = 0;
};

// only models with type "mod_brush" have this data
struct brushdata_t
{
    worldbrushdata_t *pShared;
    int firstmodelsurface, nummodelsurfaces;

    unsigned short renderHandle;
    unsigned short firstnode;
};

// only models with type "mod_sprite" have this data
struct spritedata_t
{
    int numframes;
    int width;
    int height;
    CEngineSprite *sprite;
};

struct model_t
{
    FileNameHandle_t fnHandle;
    char szName[6];

    int nLoadFlags;   // mark loaded/not loaded
    int nServerCount; // marked at load

    modtype_t type;
    int flags; // MODELFLAG_???

    // volume occupied by the model graphics
    Vector mins, maxs;
    float radius;

    union {
        brushdata_t brush;
        MDLHandle_t studio;
        spritedata_t sprite;
    };
};

class SurfInfo
{
  public:
    // Shape of the surface.
    Vector m_Verts[MAX_SURFINFO_VERTS];
    unsigned long m_nVerts;

    // Plane of the surface.
    VPlane m_Plane;

    // For engine use only..
    void *m_pEngineData;
};

struct LightShadowZBufferSample_t
{
    float m_flTraceDistance; // how far we traced. 0 = invalid
    float m_flHitDistance;   // where we hit
};


struct RayDispOutput_t
{
    short ndxVerts[4]; // 3 verts and a pad
    float u, v;        // the u, v paramters (edgeU = v1 - v0, edgeV = v2 - v0)
    float dist;        // intersection distance
};

struct decal_t
{
    decal_t *pnext;         // linked list for each surface
    decal_t *pDestroyList;  //
    SurfaceHandle_t surfID; // Surface id for persistence / unlinking
    IMaterial *material;
    int shaderID;
    DispDecalHandle_t m_DispDecal; // Handle to displacement decals associated with this

    // FIXME:
    // make dx and dy in decal space and get rid of position, so that
    // position can be rederived from the decal basis.
    Vector position; // location of the decal center in world space.
    Vector saxis;    // direction of the s axis in world space
    float dx;        // Offsets into surface texture (in texture coordinates, so we don't need floats)
    float dy;
    float scale;       // Pixel scale
    short flags;       // Decal flags  DECAL_*		!!!SAVED AS A BYTE (SEE HOST_CMD.C)
    short entityIndex; // Entity this is attached to
    int m_Size;        // size of decal, used for rejecting on dispinfo planes

    // NOTE: The following variables are dynamic variables.
    // We could put these into a separate array and reference them
    // by index to reduce memory costs of this...
    float fadeDuration; // Negative value means to fade in
    float fadeStartTime;
    color32 color;
    void *userdata; // For player decals only, decal index ( first player at slot 1 )

    unsigned short m_iDecalPool; // index into the decal pool.
    int m_iSortTree;             // MaterialSort tree id
    int m_iSortMaterial;         // MaterialSort id.
};

// NOTE: 16-bytes, preserve size/alignment - we index this alot
struct msurface1_t
{
    // garymct - are these needed? - used by decal projection code
    int textureMins[2];      // smallest unnormalized s/t position on the surface.
    short textureExtents[2]; // ?? s/t texture size, 1..512 for all non-sky surfaces

    struct
    {
        unsigned short numPrims;
        unsigned short firstPrimID; // index into primitive list if numPrims > 0
    } prims;
};

// NOTE: 32-bytes.  Aligned/indexed often
struct msurface2_t
{
    unsigned int flags; // see SURFDRAW_ #defines (only 22-bits right now)
    // These are packed in to flags now
    // unsigned char			vertCount;		// number of verts for this surface
    // unsigned char			sortGroup;		// only uses 2 bits, subdivide?
    cplane_t *plane;    // pointer to shared plane
    int firstvertindex; // look up in model->vertindices[] (only uses 17-18 bits?)
    WorldDecalHandle_t decals;
    ShadowDecalHandle_t m_ShadowDecals;              // unsigned short
    OverlayFragmentHandle_t m_nFirstOverlayFragment; // First overlay fragment on the surface (short)
    short materialSortID;
    unsigned short vertBufferIndex;

    unsigned short m_bDynamicShadowsEnabled : 1; // Can this surface receive dynamic shadows?
    unsigned short texinfo : 15;

    IDispInfo *pDispInfo; // displacement map information
    int visframe;         // should be drawn when node is crossed
};

struct mnode_t
{
    // common with leaf
    int contents; // <0 to differentiate from leafs
    // -1 means check the node for visibility
    // -2 means don't check the node for visibility

    int visframe; // node needs to be traversed if current

    mnode_t *parent;
    short area; // If all leaves below this node are in the same area, then
    // this is the area index. If not, this is -1.
    short flags;

    VectorAligned m_vecCenter;
    VectorAligned m_vecHalfDiagonal;

    // node specific
    cplane_t *plane;
    mnode_t *children[2];

    unsigned short firstsurface;
    unsigned short numsurfaces;
};

struct mleaf_t
{
  public:
    // common with node
    int contents; // contents mask
    int visframe; // node needs to be traversed if current

    mnode_t *parent;

    short area;
    short flags;
    VectorAligned m_vecCenter;
    VectorAligned m_vecHalfDiagonal;

    // leaf specific
    short cluster;
    short leafWaterDataID;

    unsigned short firstmarksurface;
    unsigned short nummarksurfaces;

    short nummarknodesurfaces;
    short unused;

    unsigned short dispListStart; // index into displist of first displacement
    unsigned short dispCount;     // number of displacements in the list for this leaf
};

struct mleafwaterdata_t
{
    float surfaceZ;
    float minZ;
    short surfaceTexInfoID;
    short firstLeafIndex;
};

// !!! if this is changed, it must be changed in asm_draw.h too !!!
struct mvertex_t
{
    Vector position;
};

// This is here for b/w compatibility with world surfaces that use
// WorldVertexTransition. We can get rid of it when we rev the engine.
#define TEXINFO_USING_BASETEXTURE2 0x0001

struct mtexinfo_t
{
    Vector4D textureVecsTexelsPerWorldUnits[2]; // [s/t] unit vectors in world space.
                                                // [i][3] is the s/t offset relative to the origin.
    Vector4D lightmapVecsLuxelsPerWorldUnits[2];
    float luxelsPerWorldUnit;
    float worldUnitsPerLuxel;
    unsigned short flags;        // SURF_ flags.
    unsigned short texinfoFlags; // TEXINFO_ flags.
    IMaterial *material;

    mtexinfo_t(mtexinfo_t const &src)
    {
        // copy constructor needed since Vector4D has no copy constructor
        memcpy(this, &src, sizeof(mtexinfo_t));
    }
};

// This is a single cache line (32 bytes)
struct msurfacelighting_t
{
    // You read that minus sign right. See the comment below.
    ColorRGBExp32 *AvgLightColor(int nLightStyleIndex) { return m_pSamples - (nLightStyleIndex + 1); }

    // Lightmap info
    short m_LightmapMins[2];
    short m_LightmapExtents[2];
    short m_OffsetIntoLightmapPage[2];

    int m_nLastComputedFrame; // last frame the surface's lightmap was recomputed
    int m_fDLightBits;        // Indicates which dlights illuminates this surface.
    int m_nDLightFrame;       // Indicates the last frame in which dlights illuminated this surface

    unsigned char m_nStyles[MAXLIGHTMAPS]; // index into d_lightstylevalue[] for animated lights
                                           // no one surface can be effected by more than 4
                                           // animated lights.

    // NOTE: This is tricky. To get this to fit in a single cache line,
    // and to save the extra memory of not having to store average light colors for
    // lightstyles that are not used, I store between 0 and 4 average light colors +before+
    // the samples, depending on how many lightstyles there are. Naturally, accessing
    // an average color for an undefined lightstyle on the surface results in undefined results.
    // 0->4 avg light colors, *in reverse order from m_nStyles* + [numstyles*surfsize]
    ColorRGBExp32 *m_pSamples;
};

struct msurfacenormal_t
{
    unsigned int firstvertnormal;
    //	unsigned short	firstvertnormal;
    // FIXME: Should I just point to the leaf here since it has this data?????????????
    //	short fogVolumeID;			// -1 if not in fog
};

struct mprimitive_t
{
    int type;
    unsigned short firstIndex;
    unsigned short indexCount;
    unsigned short firstVert;
    unsigned short vertCount;
};

struct mprimvert_t
{
    Vector pos;
    float texCoord[2];
    float lightCoord[2];
};

struct mcubemapsample_t
{
    Vector origin;
    ITexture *pTexture;
    unsigned char size; // default (mat_envmaptgasize) if 0, 1<<(size-1) otherwise.
};

struct worldbrushdata_t
{
    int numsubmodels;

    int numplanes;
    cplane_t *planes;

    int numleafs; // number of visible leafs, not counting 0
    mleaf_t *leafs;

    int numleafwaterdata;
    mleafwaterdata_t *leafwaterdata;

    int numvertexes;
    mvertex_t *vertexes;

    int numoccluders;
    doccluderdata_t *occluders;

    int numoccluderpolys;
    doccluderpolydata_t *occluderpolys;

    int numoccludervertindices;
    int *occludervertindices;

    int numvertnormalindices; // These index vertnormals.
    unsigned short *vertnormalindices;

    int numvertnormals;
    Vector *vertnormals;

    int numnodes;
    mnode_t *nodes;
    unsigned short *m_LeafMinDistToWater;

    int numtexinfo;
    mtexinfo_t *texinfo;

    int numtexdata;
    csurface_t *texdata;

    int numDispInfos;
    HDISPINFOARRAY hDispInfos; // Use DispInfo_Index to get IDispInfos..

    /*
    int         numOrigSurfaces;
    msurface_t  *pOrigSurfaces;
    */

    int numsurfaces;
    msurface1_t *surfaces1;
    msurface2_t *surfaces2;
    msurfacelighting_t *surfacelighting;
    msurfacenormal_t *surfacenormals;

    bool unloadedlightmaps;

    int numvertindices;
    unsigned short *vertindices;

    int nummarksurfaces;
    SurfaceHandle_t *marksurfaces;

    ColorRGBExp32 *lightdata;

    int numworldlights;
    dworldlight_t *worldlights;

    lightzbuffer_t *shadowzbuffers;

    // non-polygon primitives (strips and lists)
    int numprimitives;
    mprimitive_t *primitives;

    int numprimverts;
    mprimvert_t *primverts;

    int numprimindices;
    unsigned short *primindices;

    int m_nAreas;
    darea_t *m_pAreas;

    int m_nAreaPortals;
    dareaportal_t *m_pAreaPortals;

    int m_nClipPortalVerts;
    Vector *m_pClipPortalVerts;

    mcubemapsample_t *m_pCubemapSamples;
    int m_nCubemapSamples;

    int m_nDispInfoReferences;
    unsigned short *m_pDispInfoReferences;

    mleafambientindex_t *m_pLeafAmbient;
    mleafambientlighting_t *m_pAmbientSamples;
#if 0
	int			numportals;
	mportal_t	*portals;

	int			numclusters;
	mcluster_t	*clusters;

	int			numportalverts;
	unsigned short *portalverts;

	int			numclusterportals;
	unsigned short *clusterportals;
#endif
};

inline int &MSurf_FirstVertIndex(SurfaceHandle_t surfID) { return surfID->firstvertindex; }

inline int MSurf_VertCount(SurfaceHandle_t surfID) { return (surfID->flags >> SURFDRAW_VERTCOUNT_SHIFT) & 0xFF; }

inline const SurfaceHandle_t SurfaceHandleFromIndex(int surfaceIndex, const worldbrushdata_t *pData)
{
    return &pData->surfaces2[surfaceIndex];
}

inline SurfaceHandle_t SurfaceHandleFromIndex(int surfaceIndex, worldbrushdata_t *pData = host_state->worldbrush)
{
    return &pData->surfaces2[surfaceIndex];
}

extern void MOM_DrawBrushOutlineModel(CBaseEntity *baseentity, const Color &color);