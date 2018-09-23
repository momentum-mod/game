//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef DETAIL_H
#define DETAIL_H

#ifdef _WIN32
#pragma once
#endif

struct face_t;
struct tree_t;

face_t *MergeDetailTree(tree_t *worldtree, int brush_start, int brush_end);


#endif // DETAIL_H
