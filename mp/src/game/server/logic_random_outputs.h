//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#pragma once

#define NUM_RANDOM_OUTPUTS 16

class CLogicRandomOutputs : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicRandomOutputs, CLogicalEntity );

	CLogicRandomOutputs();

	void Activate();
	void Think();
	virtual bool KeyValue( const char *szKeyName, const char *szValue );

	// Input handlers
	void InputEnable( inputdata_t &inputdata );
	void InputEnableRefire( inputdata_t &inputdata );  // Private input handler, not in FGD
	void InputDisable( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );
	void InputTrigger( inputdata_t &inputdata );
	void InputCancelPending( inputdata_t &inputdata );

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_Output[ NUM_RANDOM_OUTPUTS ];
	COutputEvent m_OnSpawn;

	float m_flOnTriggerChance[ NUM_RANDOM_OUTPUTS + 1 ];	// DEFINE_AUTO_ARRAY fills this array from index 1
	
private:

	bool m_bDisabled;
	bool m_bWaitForRefire;			// Set to disallow a refire while we are waiting for our outputs to finish firing.
};
