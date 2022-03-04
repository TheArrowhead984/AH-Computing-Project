#pragma once
#include "wx/wx.h"
#include "wx/grid.h"
#include "wx/msgdlg.h"
#include "wx/notebook.h"
#include <string>

class cMain : public wxFrame
{
public:
	cMain();
	~cMain();
	//Base Panel
	wxPanel *basePanel = nullptr;
	//Title
	wxStaticText *title = nullptr;
	//Media Selection (Games, movies, shows)
	wxNotebook *mediaNB = nullptr;
	//Search type (Filter, search, random)
	wxNotebook *searchTypeNB = nullptr;
	//Sub pages
	wxScrolledWindow *movieFiltering = nullptr;
	wxPanel *movieRandom = nullptr;
	//UI elements for filtering
	wxStaticText *movieStyleHead = nullptr;
	wxChoice *movieStyleCombo = nullptr;
	wxStaticText *movieFeatureHead = nullptr;
	wxChoice *movieFeatureCombo = nullptr;
	wxStaticText *movieAgeHead = nullptr;
	wxChoice *movieAgeCombo = nullptr;
	wxButton *applyFiltersBtn = nullptr;
	wxButton *resetFiltersBtn = nullptr;
	//UI elements for random
	wxStaticText *movieTitle = nullptr;
	wxStaticBitmap *moviePoster = nullptr;
	wxStaticText *movieDescHeader = nullptr;
	wxStaticText *movieDesc = nullptr;
	wxButton *randomizerButton = nullptr;

	void applyFilters(wxCommandEvent &evt);
	void resetFilters(wxCommandEvent &evt);
	void imageClicked(wxCommandEvent &evt);
	void styleClicked(wxCommandEvent &evt);
	void newRandomMovie(wxCommandEvent &evt);
	void runFilterQuery(std::string mediaType, bool filterStyle, bool filterFeature, bool filterAge, wxString selectedStyle, wxString selectedFeature, wxString selectedAge);
	void runRandomQuery(std::string mediaType, bool random, int movieID);

	wxDECLARE_EVENT_TABLE();
};