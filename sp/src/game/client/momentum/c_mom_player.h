
#include "cbase.h"



class C_MomentumPlayer : public C_BasePlayer
{
public:
    DECLARE_CLASS(C_MomentumPlayer, C_BasePlayer);
    
    C_MomentumPlayer();
    ~C_MomentumPlayer();

    DECLARE_CLIENTCLASS();

private:
    friend class CMomentumGameMovement;

};