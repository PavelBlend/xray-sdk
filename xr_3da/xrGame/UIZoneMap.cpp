#include "stdafx.h"
#include "UIZoneMap.h"

#include "HUDManager.h"


#include "InfoPortion.h"
#include "PDA.h"

#include "Grenade.h"
#include "Level.h"
#include "game_cl_base.h"

#include "Actor.h"
#include "ai_space.h"
#include "game_graph.h"

#include "ui/UIMap.h"
#include "ui/UIXmlInit.h"
//////////////////////////////////////////////////////////////////////////

CUIZoneMap::CUIZoneMap()
{}

CUIZoneMap::~CUIZoneMap()
{
	
}

void CUIZoneMap::Init()
{

	CUIXml uiXml;
	bool xml_result			= uiXml.Init(CONFIG_PATH, UI_PATH, "zone_map.xml");
	R_ASSERT3(xml_result, "xml file not found", "zone_map.xml");

	// load map backgroundwwwwwwwwwwwww
	CUIXmlInit xml_init;
	xml_init.InitStatic			(uiXml, "minimap:background", 0, &m_background);

	if(IsGameTypeSingle()){
		xml_init.InitStatic			(uiXml, "minimap:background:dist_text", 0, &m_pointerDistanceText);
		m_background.AttachChild	(&m_pointerDistanceText);
	}

	xml_init.InitStatic(uiXml, "minimap:level_frame", 0, &m_clipFrame);

	xml_init.InitStatic(uiXml, "minimap:center", 0, &m_center);
	
	
	m_activeMap						= xr_new<CUIMiniMap>();
	m_clipFrame.AttachChild			(m_activeMap);
	m_activeMap->SetAutoDelete		(true);

	m_activeMap->EnableHeading		(true);  
	xml_init.InitStatic				(uiXml, "minimap:compass", 0, &m_compass);

//	m_background.AttachChild(&m_compass);

	m_clipFrame.AttachChild			(&m_center);
	m_center.SetWndPos				(m_clipFrame.GetWidth()/2,m_clipFrame.GetHeight()/2);
}

void CUIZoneMap::Render			()
{
	m_clipFrame.Draw	();
	m_background.Draw	();
	m_compass.Draw		();
}

void CUIZoneMap::SetHeading		(float angle)
{
	m_activeMap->SetHeading(angle);
	m_compass.SetHeading(angle);
};

void CUIZoneMap::UpdateRadar		(Fvector pos)
{
	m_clipFrame.Update();
	m_background.Update();
	m_activeMap->SetActivePoint( pos );

	if(IsGameTypeSingle()){
		if(m_activeMap->GetPointerDistance()>0.5f){
			string64	str;
			sprintf_s		(str,"%.1f m.",m_activeMap->GetPointerDistance());
			m_pointerDistanceText.SetText(str);
		}else{
			m_pointerDistanceText.SetText("");
		}
	}
}

bool CUIZoneMap::ZoomIn()
{
	return true;
}

bool CUIZoneMap::ZoomOut()
{
	return true;
}

void CUIZoneMap::SetupCurrentMap()
{
	CInifile* pLtx				= pGameIni;

	if(!pLtx->section_exist(Level().name()))
		pLtx							= Level().pLevel;

	m_activeMap->Init				(Level().name(),*pLtx,"hud\\default");

	Frect r;
	m_clipFrame.GetAbsoluteRect		(r);
	m_activeMap->SetClipRect		(r);
	
	Fvector2						wnd_size;
	float zoom_factor				= float(m_clipFrame.GetWndRect().width())/100.0f;
	
	LPCSTR ln						= Level().name().c_str();
	if(	pGameIni->section_exist(ln) )
	{
		if(pGameIni->line_exist(ln, "minimap_zoom"))
			zoom_factor *= pGameIni->r_float(ln, "minimap_zoom");
	}else
	if(g_pGameLevel->pLevel->section_exist("minimap_zoom"))
	{
		zoom_factor *= g_pGameLevel->pLevel->r_float("minimap_zoom", "value");
	}
	wnd_size.x						= m_activeMap->BoundRect().width()*zoom_factor;
	wnd_size.y						= m_activeMap->BoundRect().height()*zoom_factor;
	m_activeMap->SetWndSize			(wnd_size);
}

void CUIZoneMap::OnSectorChanged(int sector)
{
	if(!g_pGameLevel->pLevel->section_exist("sub_level_map") )
		return;
	u8			map_idx = u8(-1);
	string64	s_sector;
	sprintf	(s_sector, "%d", sector);
	
	if(!g_pGameLevel->pLevel->line_exist("sub_level_map", s_sector) )
		return;

	map_idx		= g_pGameLevel->pLevel->r_u8("sub_level_map", s_sector);
	if(m_current_map_idx == map_idx)
		return;

	m_current_map_idx = map_idx;

	string_path sub_texture;
	sprintf(sub_texture,"%s#%d", m_activeMap->m_texture.c_str(), m_current_map_idx);
	
	if(map_idx==u8(-1))
		sprintf(sub_texture,"%s", m_activeMap->m_texture.c_str());

	m_activeMap->InitTextureEx(sub_texture, m_activeMap->m_shader_name.c_str());
}
