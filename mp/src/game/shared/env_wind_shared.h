//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements visual effects entities: sprites, beams, bubbles, etc.
//
// $NoKeywords: $
//=============================================================================//

#ifndef ENV_WIND_SHARED_H
#define ENV_WIND_SHARED_H

#include "utllinkedlist.h"
#include "vstdlib/random.h"
#include "tier0/dbg.h"
#include "mathlib/vector.h"
#include <float.h>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CSoundPatch;

//-----------------------------------------------------------------------------
// Class used to help store events that occurred over time
//-----------------------------------------------------------------------------
template <class T, class I>
class CTimedEventQueue
{
public:
	// The time passed in here represents the amount of time the queue stores
	CTimedEventQueue( float flMaxTime );

	// Adds an event to the queue, will pop off stale events from the queue
	// NOTE: All events added to the queue must monotonically increase in time!
	I PushEvent( float flTime, const T &data );

	// Grabs the last event that happened before or at the specified time
	I GetEventIndex( float flTime ) const;

	// Gets event information
	float GetEventTime( I i ) const;
	const T &GetEventData( I i ) const;

private:
	struct QueueEntry_t
	{
		float	m_flTime;
		T		m_Data;
	};

	float m_flQueueHeadTime;
	float m_flMaxTime;
	CUtlLinkedList< T, I >	m_Queue;
};


//-----------------------------------------------------------------------------
// The time passed in here represents the amount of time the queue stores
//-----------------------------------------------------------------------------
template <class T, class I>
CTimedEventQueue<T,I>::CTimedEventQueue( float flMaxTime ) : m_flMaxTime(flMaxTime)
{
	// The length of time of events in the queue must be reasonable
	Assert( m_flMaxTime > 0.0f );
	m_flQueueHeadTime = -FLT_MAX;
}


//-----------------------------------------------------------------------------
// Adds an event to the queue, will pop off stale events from the queue
//-----------------------------------------------------------------------------
template <class T, class I>
I CTimedEventQueue<T,I>::PushEvent( float flTime, const T &data )
{
	Assert( m_flQueueHeadTime <= flTime );
	m_flQueueHeadTime = flTime;

	// First push the event...
	I idx = m_Queue.AddToHead();
	m_Queue[idx].m_flTime = flTime;
	m_Queue[idx].m_Data = data;
	
	// Then retire stale events...
	I i = m_Queue.Tail();
	while (m_Queue[i].m_flTime < m_flQueueHeadTime - m_flMaxTime )
	{
		I prev = m_Queue.Prev(i);
		Assert( prev != m_Queue.InvalidIndex() );
		m_Queue.Remove(i);
		i = prev;
	}

	return idx;
}


//-----------------------------------------------------------------------------
// Grabs the last event that happened before or at the specified time
//-----------------------------------------------------------------------------
template <class T, class I>
I CTimedEventQueue<T,I>::GetEventIndex( float flTime ) const
{
	// This checks for a request that fell off the queue
	Assert( (flTime >= m_flQueueHeadTime - m_flMaxTime) && (flTime <= m_flQueueHeadTime) );
	
	// Then retire stale events...
	I i = m_Queue.Head();
	while( m_Queue[i].m_flTime > flTime )
	{
		i = m_Queue.Next(i);
		Assert( i != m_Queue.InvalidIndex() );
	}

	return i;
}


//-----------------------------------------------------------------------------
// Gets event information
//-----------------------------------------------------------------------------
template <class T, class I>
inline float CTimedEventQueue<T,I>::GetEventTime( I i )	const
{
	return m_Queue[i].m_flTime;
}

template <class T, class I>
inline const T &CTimedEventQueue<T,I>::GetEventData( I i ) const
{
	return m_Queue[i].m_Data;
}


//-----------------------------------------------------------------------------
// Implementation of the class that computes windspeed
//-----------------------------------------------------------------------------
class CEnvWindShared
{
public:
	DECLARE_CLASS_NOBASE( CEnvWindShared );
	DECLARE_EMBEDDED_NETWORKVAR();
	
	CEnvWindShared();
	~CEnvWindShared();

	void Init( int iEntIndex, int iRandomSeed, float flTime, int iWindDir, float flInitialWindSpeed );

	// Method to update the wind speed
	// Time passed in here is global time, not delta time
	// The function returns the time at which it must be called again
	float WindThink( float flTime );

    void SetLocation(const Vector &location);

	// FIXME: These really should be private
	CNetworkVar( float, m_flStartTime );

	CNetworkVar( int, m_iWindSeed );		// random number seed...

	CNetworkVar( int, m_iMinWind );			// the slowest the wind can normally blow
	CNetworkVar( int, m_iMaxWind );			// the fastest the wind can normally blow
    CNetworkVar(int, m_windRadius); // the radius this entity affects with its windiness, so a map can have multiple
	CNetworkVar( float, m_windRadiusInner );		// the inner-radius for noticable distance fading
    CNetworkVar(int, m_iMinGust);   // the slowest that a gust can be
	CNetworkVar( int, m_iMaxGust );			// the fastest that a gust can be

	CNetworkVar( float, m_flMinGustDelay );	// min time between gusts
	CNetworkVar( float, m_flMaxGustDelay );	// max time between gusts

	CNetworkVar( float, m_flGustDuration );	// max time between gusts

	CNetworkVar( int, m_iGustDirChange );	// max number of degrees wind dir changes on gusts.
    CNetworkVector(m_location);         // The location of this wind controller

	int m_iszGustSound;		// name of the wind sound to play for gusts.
	int m_iWindDir;			// wind direction (yaw)
	float m_flWindSpeed;	// the wind speed

    Vector m_currentWindVector; // For all the talk of proper prediction, we ended up just storing and returning through
                                // a static vector.  Now we can have multiple env_wind, so we need this in here.
    Vector m_CurrentSwayVector;
    Vector m_PrevSwayVector;

	CNetworkVar( float, m_flTreeSwayScale );

	CNetworkVar( int, m_iInitialWindDir );
	CNetworkVar( float, m_flInitialWindSpeed );

#ifndef CLIENT_DLL
	COutputEvent m_OnGustStart;
	COutputEvent m_OnGustEnd;
#endif

    CEnvWindShared &operator=(const CEnvWindShared &other)
    {
        m_flStartTime = other.m_flStartTime;
        m_iWindSeed = other.m_iWindSeed;
        m_iMinWind = other.m_iMinWind;
        m_iMaxWind = other.m_iMaxWind;
        m_windRadius = other.m_windRadius;
        m_iMinGust = other.m_iMinGust;
        m_iMaxGust = other.m_iMaxGust;
        m_flMinGustDelay = other.m_flMinGustDelay;
        m_flMaxGustDelay = other.m_flMaxGustDelay;
        m_flGustDuration = other.m_flGustDuration;
        m_iGustDirChange = other.m_iGustDirChange;
        m_location = other.m_location;
        m_iInitialWindDir = other.m_iInitialWindDir;
        m_flInitialWindSpeed = other.m_flInitialWindSpeed;

        return *this;
    }

private:
	struct WindAveEvent_t
	{
		float m_flStartWindSpeed;	// the wind speed at the time of the event
		float m_flAveWindSpeed;		// the average wind speed of the event
	};

	struct WindVariationEvent_t
	{
		float m_flWindAngleVariation;
		float m_flWindSpeedVariation;
	};

	void ComputeWindVariation( float flTime );

	// Updates the wind sound
    void UpdateWindSound(float flTotalWindSpeed);
    void UpdateTreeSway(float flTime);

	float m_flVariationTime;
    float m_flSwayTime;
	float	m_flSimTime;		// What's the time I last simulated up to?
	float	m_flSwitchTime;		// when do I actually switch from gust to not gust
	float	m_flAveWindSpeed;	// the average wind speed
	bool	m_bGusting;			// is the wind gusting right now?

	float m_flWindAngleVariation;
	float m_flWindSpeedVariation;

	int m_iEntIndex;

	// Used to generate random numbers
	CUniformRandomStream m_Stream;

	// NOTE: In order to make this algorithm independent of calling frequency
	// I have to decouple the stream used to generate average wind speed
	// and the stream used to generate wind variation since they are
	// simulated using different timesteps
	CUniformRandomStream m_WindVariationStream;

	// Used to generate the wind sound...
	CSoundPatch *m_pWindSound;

	// Event history required for prediction
	CTimedEventQueue< WindAveEvent_t, unsigned short >	m_WindAveQueue;
	CTimedEventQueue< WindVariationEvent_t, unsigned short > m_WindVariationQueue;

  public:
    bool IsGusting() const { return m_bGusting; }

  private:
	CEnvWindShared( const CEnvWindShared & ); // not defined, not accessible
};

//-----------------------------------------------------------------------------
inline void CEnvWindShared::SetLocation(const Vector &location) { m_location = location; }

//-----------------------------------------------------------------------------
// Method to sample the wind speed at a particular location
//-----------------------------------------------------------------------------
Vector GetWindspeedAtLocation(const Vector &location);

//-----------------------------------------------------------------------------
// Method to sample the windspeed at a particular time
//-----------------------------------------------------------------------------
void GetWindspeedAtTime( float flTime, Vector &vecVelocity );


//-----------------------------------------------------------------------------
// Method to reset windspeed..
//-----------------------------------------------------------------------------
void ResetWindspeed();


#endif // ENV_WIND_SHARED_H

