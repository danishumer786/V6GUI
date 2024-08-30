#pragma once

#include "wx/wx.h"

#include "MainLaserControllerInterface.h"


class SettingsPage_Base : public wxScrolledWindow{

public:
	SettingsPage_Base(
		std::shared_ptr<MainLaserControllerInterface> _lc,
		wxWindow* parent
	);

	virtual void Init() {};
	virtual void RefreshAll() {};
	virtual void RefreshStrings() {};
	virtual void RefreshVisibility() {};
	virtual void RefreshControlEnabled() {};


protected:
	std::shared_ptr<MainLaserControllerInterface> lc;

};
