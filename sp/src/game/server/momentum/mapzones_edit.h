#define BUILDSTAGE_NONE         0
#define BUILDSTAGE_START        1
//#define BUILDSTAGE_ROTATE     2
#define BUILDSTAGE_END          2
#define BUILDSTAGE_HEIGHT       3


extern ConVar mom_zone_edit;

class CMapzoneEdit
{
public:
    void Build( Vector *aimpos, int type, int forcestage = BUILDSTAGE_NONE );

    // Draw lines and update the zone height.
    void Update();

    void Reset() { mom_zone_edit.SetValue( "0" ); m_nBuildStage = BUILDSTAGE_NONE; m_bEditing = false; };

    int GetBuildStage() { return m_nBuildStage; };
    void SetBuildStage( int stage ) { m_nBuildStage = stage; };

    void IncreaseZoom( float dist ) { m_flReticleDist = fminf( m_flReticleDist + dist, 2048.0f ); };
    void DecreaseZoom( float dist ) { m_flReticleDist = fmaxf( m_flReticleDist - dist, 16.0f ); };
    float GetZoom() { return m_flReticleDist; };
    float SetZoom( float dist ) { m_flReticleDist = fmaxf( fminf( dist, 2048.0f ), 16.0f ); };
    
    // Placeholder, move this somewhere else if other files start using the zone types.
    int GetEntityZoneType( CBaseEntity *pEnt );
    
    // start/stop/stage
    int ShortNameToZoneType( const char *in );

    // Placeholder
    bool ZoneTypeToClass( int type, char *dest );

private:
    int m_nBuildStage = BUILDSTAGE_NONE;
    float m_flReticleDist = 256.0f;
    Vector m_vecBuildStart;
    Vector m_vecBuildEnd;
    bool m_bEditing = false;

    float GetZoneHeightToPlayer( CBasePlayer *pPlayer );
    void SetZoneProps( CBaseEntity *pEnt );
    float SnapToGrid( float fl, float gridsize );
    void VectorSnapToGrid( Vector *dest, float gridsize );
    void DrawReticle( Vector *pos, float retsize );
};

extern CMapzoneEdit g_MapzoneEdit;