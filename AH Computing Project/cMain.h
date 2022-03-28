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
	wxStaticText *movieReleaseHead = nullptr;
	wxChoice *movieReleaseStartVal = nullptr;
	wxChoice *movieReleaseEndVal = nullptr;
	wxStaticText *movieDurationHead = nullptr;
	wxChoice *movieDurationStartVal = nullptr;
	wxChoice *movieDurationEndVal = nullptr;
	wxButton *movieApplyFiltersBtn = nullptr;
	wxButton *movieResetFiltersBtn = nullptr;
	wxBitmapButton *movieSortDirection = nullptr;
	wxChoice *movieSortChoice = nullptr;
	//UI elements for random
	wxStaticText *movieTitle = nullptr;
	wxStaticBitmap* movieAge = nullptr;
	wxStaticBitmap *moviePoster = nullptr;
	wxStaticText *movieDescHeader = nullptr;
	wxStaticText *movieDesc = nullptr;
	wxButton *randomizerButton = nullptr;

	void movieApplyFilters(wxCommandEvent &evt);
	void movieResetFilters(wxCommandEvent &evt);
	void movieImageClicked(wxCommandEvent &evt);
	void movieStyleClicked(wxCommandEvent &evt);
	void movieFeatureClicked(wxCommandEvent &evt);
	void newRandomMovie(wxCommandEvent &evt);
	void changeSortDirection(wxCommandEvent &evt);
	void sortChoiceMade(wxCommandEvent &evt);
	void runFilterQuery(std::string mediaType, wxBitmapButton *mediaTypeCovers[50], wxStaticText *mediaTypeInfo[50], bool filterStyle=false, bool filterFeature=false, bool filterAge=false, bool filterRelease=false, bool filterDuration=false, wxString selectedStyle = "ANY", wxString selectedFeature = "ANY", wxString selectedAge = "ANY", wxString selectedReleaseStart = "ANY", wxString selectedReleaseEnd="ANY", wxString selectedDurationStart = "ANY", wxString selectedDurationEnd = "ANY", wxString sortedField="Title", bool sortDirection=false);
	void runRandomQuery(std::string mediaType, bool random=false, int movieID=1);

	wxDECLARE_EVENT_TABLE();
};