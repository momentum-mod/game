//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#pragma once

#include "entityinput.h"
#include "entityoutput.h"
#include "eventqueue.h"

class CLogicRelay : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicRelay, CLogicalEntity );

	void Activate();
	void Think();

	// Input handlers
	void InputEnable( inputdata_t &inputdata );
	void InputEnableRefire( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );
	void InputTrigger( inputdata_t &inputdata );
	void InputTriggerWithParameter( inputdata_t &inputdata );
	void InputCancelPending( inputdata_t &inputdata );

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_OnTrigger;
	COutputVariant m_OnTriggerParameter;
	COutputEvent m_OnSpawn;

	bool IsDisabled( void ){ return m_bDisabled; }
	
private:

	bool m_bDisabled;
	bool m_bWaitForRefire;			// Set to disallow a refire while we are waiting for our outputs to finish firing.
};

struct LogicRelayQueueInfo_t
{
	DECLARE_SIMPLE_DATADESC();

	bool TriggerWithParameter;
	CBaseEntity *pActivator;
	variant_t value;
	int outputID;
};

class CLogicRelayQueue : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicRelayQueue, CLogicalEntity );

	// Input handlers
	void InputEnable( inputdata_t &inputdata );
	void InputEnableRefire( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );
	void InputTrigger( inputdata_t &inputdata );
	void InputTriggerWithParameter( inputdata_t &inputdata );
	void InputCancelPending( inputdata_t &inputdata );
	void InputClearQueue( inputdata_t &inputdata );

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_OnTrigger;
	COutputVariant m_OnTriggerParameter;

	bool IsDisabled( void ){ return m_bDisabled; }

	void HandleNextQueueItem();
	void AddQueueItem(CBaseEntity *pActivator, int outputID, variant_t &value);
	void AddQueueItem(CBaseEntity *pActivator, int outputID);

	int		DrawDebugTextOverlays( void );
	
private:

	bool m_bDisabled;
	bool m_bWaitForRefire;			// Set to disallow a refire while we are waiting for our outputs to finish firing.

	int m_iMaxQueueItems;
	bool m_bDontQueueWhenDisabled; // Don't add to queue while disabled, only when waiting for refire
	CUtlVector<LogicRelayQueueInfo_t> m_QueueItems;
};
