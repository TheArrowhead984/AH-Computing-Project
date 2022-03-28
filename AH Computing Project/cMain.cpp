//Library inclusions
#include "cMain.h"
#include "movie.h"
#include "mysql.h"
#include <random>
#include <sstream>

//Define event table
wxBEGIN_EVENT_TABLE(cMain, wxFrame)
EVT_BUTTON(1, movieApplyFilters)
EVT_BUTTON(2, movieResetFilters)
EVT_BUTTON(3, newRandomMovie)
EVT_BUTTON(4, changeSortDirection)
EVT_CHOICE(5, sortChoiceMade)
wxEND_EVENT_TABLE()



/* <------------------Define global variables------------------>*/

//SQL stuff
int qstate;
MYSQL *conn;
MYSQL_ROW row;
MYSQL_RES *res;

//Scaling values
float offsetY = 49.0f;
float choiceOffset = 5.0f;
float headOffset = 27.5f;
int titleFontSize = 30;
int NBfontSize = 20;
int headingFontSize = 20;
int btnFontSize = 18;
int frameX = 0;
int frameY = 0;
double frameXRatio = 0.0;
double frameYRatio = 0.0;
int coverX = 240;
int coverY = 360;

//Sorting Variables
std::string movieSortDataType = "Alpha";
bool movieSort = false; //boolean for which direction to sort (ascending (false) or descending (true))
bool movieSortType = true; //boolean for what data type is being sorted (int (false) or string (true))

//Result Variables
int noMovies = 0;
int noResults = 0;
int noStyles = 0;
int noFeatures = 0;
wxBitmapButton *movieCovers[50];
wxStaticText *movieInfo[50];
wxButton *styleList[8];
wxButton *featureList[4];
int previousMovie = 0;
movie *movies[50];

/* <------------------Functions------------------>*/

/* <-----Function for rescaling images----->*/
wxBitmap scaleImage(wxBitmap Bitmap, int newX, int newY)
{
	wxImage image = Bitmap.ConvertToImage();
	image.Rescale(newX, newY, wxIMAGE_QUALITY_HIGH);
	return image;
}

/* <-----Function for generating a random integer in a range----->*/
int randInt(int start, int end)
{
	std::random_device rd; //Get random number
	std::mt19937 gen(rd()); //Seed generation
	std::uniform_int_distribution<> distr(start, end); //Define range
	return distr(gen);
}

/* <-----Function that checks how many entries are in the database----->*/
int getNumberOfEntries(std::string mediaType) {
	wxString queryStr = "SELECT COUNT(*) FROM " + mediaType;
	qstate = mysql_query(conn, queryStr); //Run query
	if (!qstate) //If query executes without error
	{
		res = mysql_store_result(conn);
		if (res->row_count > 0) {
			while (row = mysql_fetch_row(res)) {
				return std::stoi(row[0]);
			}
		}
	}
}

/* <-----Procedure that fills in details about each movie----->*/
void initializeMovies(int movieID) {

	qstate = mysql_query(conn, ("SELECT * FROM movies WHERE movieID = " + std::to_string(movieID + 1)).c_str()); //Run query
	if (!qstate) //If query executes without error
	{
		res = mysql_store_result(conn);
		if (res->row_count > 0) //If the query returns results
		{
			while (row = mysql_fetch_row(res)) {
				std::string styles[8];
				std::string features[4];
				std::stringstream unsplitStyles(row[7]); //Prepare string for splitting (Assign stringstream data type)
				int styleCounter = 0; //Initialise/reset counter
				while (unsplitStyles.good()) //While EOF not hit
				{
					std::string singleLine;
					getline(unsplitStyles, singleLine, '/'); //Split into seperate styles ('/' is delimiter)
					styles[styleCounter] = singleLine;
					styleCounter++;
				}
				std::stringstream unsplitFeats(row[6]); //Prepare string for splitting (Assign stringstream data type)
				int featCounter = 0; //Initialise/reset counter
				while (unsplitFeats.good()) //While EOF not hit
				{
					std::string singleLine;
					getline(unsplitFeats, singleLine, '/'); //Split into seperate features ('/' is delimiter)
					features[featCounter] = singleLine;
					featCounter++;
				}
				movies[movieID] = new movie(std::stoi(row[0]), new wxBitmap("Resources/Gallery/movies/" + std::string(row[1]) + "/Cover.png", wxBITMAP_TYPE_PNG), std::string(row[1]), std::string(row[2]), std::stoi(row[3]), std::string(row[4]), std::string(row[5]), features, styles, std::string(row[8]));
			}
		}
	}
}

/* <------------------Event Procedures------------------>*/

/* <-----Procedure that passes selections from choice widgets to run filter query procedure----->*/
void cMain::movieApplyFilters(wxCommandEvent &evt)
{
	//Convert selections to strings
	wxString selectedStyle = movieStyleCombo->GetStringSelection();
	wxString selectedFeature = movieFeatureCombo->GetStringSelection();
	wxString selectedAge = movieAgeCombo->GetStringSelection();
	wxString selectedReleaseStart = movieReleaseStartVal->GetStringSelection();
	wxString selectedReleaseEnd = movieReleaseEndVal->GetStringSelection();
	wxString selectedDurationStart = movieDurationStartVal->GetStringSelection();
	wxString selectedDurationEnd = movieDurationEndVal->GetStringSelection();
	wxString sortedField = movieSortChoice->GetStringSelection();
	//Initialise booleans
	bool filterStyle = false;
	bool filterFeature = false;
	bool filterAge = false;
	bool filterRelease = false;
	bool filterDuration = false;
	bool sort = false;

	//Set whether the user is filtering attribute
	if (selectedStyle != wxT("ANY")) {
		filterStyle = true;
	}
	if (selectedFeature != wxT("ANY")) {
		filterFeature = true;
	}
	if (selectedAge != wxT("ANY")) {
		filterAge = true;
	}
	if (selectedReleaseStart != wxT("ANY") && selectedReleaseEnd != wxT("ANY")) {
		filterRelease = true;
	}
	if (selectedDurationStart != wxT("ANY") && selectedDurationEnd != wxT("ANY")) {
		filterDuration = true;
	}
	runFilterQuery("movies", movieCovers, movieInfo, filterStyle, filterFeature, filterAge, filterRelease, filterDuration, selectedStyle, selectedFeature, selectedAge, selectedReleaseStart, selectedReleaseEnd, selectedDurationStart, selectedDurationEnd, sortedField, movieSort); //Plug info into func
}

/* <-----Procedure that resets filtering----->*/
void cMain::movieResetFilters(wxCommandEvent &evt)
{
	//Return selections to default
	movieStyleCombo->SetSelection(0);
	movieFeatureCombo->SetSelection(0);
	movieAgeCombo->SetSelection(0);
	movieReleaseStartVal->SetSelection(0);
	movieReleaseEndVal->SetSelection(0);
	movieDurationStartVal->SetSelection(0);
	movieDurationEndVal->SetSelection(0);
	movieSortChoice->SetSelection(0);
	movieSortDataType = "Alpha";
	movieSort = false;
	movieSortType = true;
	movieSortDirection->SetBitmap(scaleImage(wxBitmap("Resources/Gallery/UI/Sorting/" + movieSortDataType + " - Ascending.png", wxBITMAP_TYPE_PNG), (int)(34 * frameXRatio + 0.5), (int)(34 * frameYRatio + 0.5)));
	runFilterQuery("movies", movieCovers, movieInfo); //Run filter query without any filters applied
}

/* <-----Procedure that shows info for a specific film----->*/
void cMain::movieImageClicked(wxCommandEvent &evt)
{
	searchTypeNB->ChangeSelection(1); //Change selected page to the random page
	runRandomQuery("movies", false, evt.GetId() - 6);
}

/* <-----Procedure that filters for a style shown on the random page----->*/
void cMain::movieStyleClicked(wxCommandEvent &evt)
{
	searchTypeNB->ChangeSelection(0);
	wxButton *btn = static_cast<wxButton *>(evt.GetEventObject()); //Get a reference to the button that triggered this event
	movieStyleCombo->SetSelection(movieStyleCombo->FindString(btn->GetLabel())); //Set selected style filter to the text on the button the user clicked
	movieFeatureCombo->SetSelection(0);
	movieAgeCombo->SetSelection(0);
	movieReleaseStartVal->SetSelection(0);
	movieReleaseEndVal->SetSelection(0);
	movieDurationStartVal->SetSelection(0);
	movieDurationEndVal->SetSelection(0);
	movieSortChoice->SetSelection(0);
	movieSortDataType = "Alpha";
	movieSort = false;
	movieSortType = true;
	movieSortDirection->SetBitmap(scaleImage(wxBitmap("Resources/Gallery/UI/Sorting/" + movieSortDataType + " - Ascending.png", wxBITMAP_TYPE_PNG), (int)(34 * frameXRatio + 0.5), (int)(34 * frameYRatio + 0.5)));
	runFilterQuery("movies", movieCovers, movieInfo, true, false, false, false, false, btn->GetLabel()); //Filter for the style the user selected
}

/* <-----Procedure that filters for a feature shown on the random page----->*/
void cMain::movieFeatureClicked(wxCommandEvent &evt)
{
	searchTypeNB->ChangeSelection(0);
	wxButton *btn = static_cast<wxButton *>(evt.GetEventObject()); //Get a reference to the button that triggered this event
	movieStyleCombo->SetSelection(0);
	movieFeatureCombo->SetSelection(movieFeatureCombo->FindString(btn->GetLabel())); //Set selected feature filter to the text on the button the user clicked
	movieAgeCombo->SetSelection(0);
	movieReleaseStartVal->SetSelection(0);
	movieReleaseEndVal->SetSelection(0);
	movieDurationStartVal->SetSelection(0);
	movieDurationEndVal->SetSelection(0);
	movieSortChoice->SetSelection(0);
	movieSortDataType = "Alpha";
	movieSort = false;
	movieSortType = true;
	movieSortDirection->SetBitmap(scaleImage(wxBitmap("Resources/Gallery/UI/Sorting/" + movieSortDataType + " - Ascending.png", wxBITMAP_TYPE_PNG), (int)(34 * frameXRatio + 0.5), (int)(34 * frameYRatio + 0.5)));
	runFilterQuery("movies", movieCovers, movieInfo, false, true, false, false, false, "", btn->GetLabel()); //Filter for the style the user selected
}

/* <-----Procedure that runs the random query procedure, this is done because of event handling----->*/
void cMain::newRandomMovie(wxCommandEvent &evt)
{
	runRandomQuery("movies", true);
}

/* <-----Procedure for updating sort direction when the icon is clicked----->*/
void cMain::changeSortDirection(wxCommandEvent &evt)
{
	if (movieSort) {
		movieSortDirection->SetBitmap(scaleImage(wxBitmap("Resources/Gallery/UI/Sorting/" + movieSortDataType + " - Ascending.png", wxBITMAP_TYPE_PNG), (int)(34 * frameXRatio + 0.5), (int)(34 * frameYRatio + 0.5)));
		movieSort = false;
	}
	else {
		movieSortDirection->SetBitmap(scaleImage(wxBitmap("Resources/Gallery/UI/Sorting/" + movieSortDataType + " - Descending.png", wxBITMAP_TYPE_PNG), (int)(34 * frameXRatio + 0.5), (int)(34 * frameYRatio + 0.5)));
		movieSort = true;
	}
}

/* <-----Procedure for updating sort direction image when changing what field to sort by---->*/
void cMain::sortChoiceMade(wxCommandEvent &evt)
{
	if (movieSortChoice->GetStringSelection() == "Title" || movieSortChoice->GetStringSelection() == "Director") {
		movieSortDataType = "Alpha";
	}
	else {
		movieSortDataType = "Number";
	}
	if (movieSort) {
		movieSortDirection->SetBitmap(scaleImage(wxBitmap("Resources/Gallery/UI/Sorting/" + movieSortDataType + " - Descending.png", wxBITMAP_TYPE_PNG), (int)(34 * frameXRatio + 0.5), (int)(34 * frameYRatio + 0.5)));
	}
	else {
		movieSortDirection->SetBitmap(scaleImage(wxBitmap("Resources/Gallery/UI/Sorting/" + movieSortDataType + " - Ascending.png", wxBITMAP_TYPE_PNG), (int)(34 * frameXRatio + 0.5), (int)(34 * frameYRatio + 0.5)));
	}
}

/* <-----Procedure that constructs and executes SQL queries from selected filters----->*/
void cMain::runFilterQuery(std::string mediaType, wxBitmapButton *mediaTypeCovers[50], wxStaticText *mediaTypeInfo[50], bool filterStyle, bool filterFeature, bool filterAge, bool filterRelease, bool filterDuration, wxString selectedStyle, wxString selectedFeature, wxString selectedAge, wxString selectedReleaseStart, wxString selectedReleaseEnd, wxString selectedDurationStart, wxString selectedDurationEnd, wxString sortedField, bool sortDirection)
{
	//Construct SQL query from user filters
	std::string queryStr = "SELECT movieID FROM " + mediaType + " WHERE ";
	if (filterStyle) {
		queryStr.append("INSTR(styles, '" + selectedStyle + "') <> 0");
		if (filterFeature || filterAge || filterRelease || filterDuration) { queryStr.append(" AND "); }
	}
	if (filterFeature) {
		queryStr.append("INSTR(features, '" + selectedFeature + "') <> 0");
		if (filterAge || filterRelease || filterDuration) { queryStr.append(" AND "); }
	}
	if (filterAge) {
		queryStr.append("ageRating = '" + selectedAge + "'");
		if (filterRelease || filterDuration) { queryStr.append(" AND "); }
	}
	if (filterRelease) {
		queryStr.append("releaseDate BETWEEN " + selectedReleaseStart + " AND " + selectedReleaseEnd);
		if (filterDuration) { queryStr.append(" AND "); }
	}
	if (filterDuration) {
		queryStr.append("duration BETWEEN '" + selectedDurationStart + "' AND '" + selectedDurationEnd + "'");
	}
	if (filterStyle != true && filterFeature != true && filterAge != true && filterRelease != true && filterDuration != true) {
		queryStr = "SELECT movieID FROM " + mediaType;
	}
	//Insertion sort movies
	if (sortedField == "Title") {
		movie *value;
		for (int i = 1; i < noMovies; i++) {
			value = movies[i];
			if (sortDirection) {
				while (i > 0 && value->title > movies[i - 1]->title) {
					movies[i] = movies[i - 1];
					i--;
				}
			}
			else {
				while (i > 0 && value->title < movies[i - 1]->title) {
					movies[i] = movies[i - 1];
					i--;
				}
			}
			movies[i] = value;
		}
	}
	else if (sortedField == "Director") {
		movie *value;
		for (int i = 1; i < noMovies; i++) {
			value = movies[i];
			if (sortDirection) {
				while (i > 0 && value->director > movies[i - 1]->director) {
					movies[i] = movies[i - 1];
					i--;
				}
			}
			else {
				while (i > 0 && value->director < movies[i - 1]->director) {
					movies[i] = movies[i - 1];
					i--;
				}
			}
			movies[i] = value;
		}
	}
	else if (sortedField == "Release Date") {
		movie *value;
		for (int i = 1; i < noMovies; i++) {
			value = movies[i];
			if (sortDirection) {
				while (i > 0 && value->releaseDate > movies[i - 1]->releaseDate) {
					movies[i] = movies[i - 1];
					i--;
				}
			}
			else {
				while (i > 0 && value->releaseDate < movies[i - 1]->releaseDate) {
					movies[i] = movies[i - 1];
					i--;
				}
			}
			movies[i] = value;
		}
	}
	else if (sortedField == "Age Rating") {
		movie *value;
		for (int i = 1; i < noMovies; i++) {
			value = movies[i];
			if (value->ageRating == "PG") {
				value->ageRating = "10";
			}
			if (movies[i - 1]->ageRating == "PG") {
				movies[i - 1]->ageRating = "10";
			}
			if (sortDirection) {
				while (i > 0 && value->ageRating > movies[i - 1]->ageRating) {
					movies[i] = movies[i - 1];
					i--;
				}
			}
			else {
				while (i > 0 && value->ageRating < movies[i - 1]->ageRating) {
					movies[i] = movies[i - 1];
					i--;
				}
			}
			movies[i] = value;

		}
		for (int i = 0; i < noMovies; i++) {
			if (movies[i]->ageRating == "10") {
				movies[i]->ageRating = "PG";
			}
		}
	}
	else if (sortedField == "Duration") {
		movie *value;
		for (int i = 1; i < noMovies; i++) {
			value = movies[i];
			if (sortDirection) {
				while (i > 0 && value->duration > movies[i - 1]->duration) {
					movies[i] = movies[i - 1];
					i--;
				}
			}
			else {
				while (i > 0 && value->duration < movies[i - 1]->duration) {
					movies[i] = movies[i - 1];
					i--;
				}
			}
			movies[i] = value;
		}
	}



	//Execute constructed query
	qstate = mysql_query(conn, queryStr.c_str());
	if (!qstate) //If query executes successfully
	{
		res = mysql_store_result(conn);
		int mediaTypeIndexes[50];
		for (int i = 0; i < 50; i++) {
			mediaTypeIndexes[i] = 0;
		}

		if (res->row_count > 0) {
			//Delete all covers currently on display from previous query
			for (int i = 0; i < noResults; i++) {
				delete mediaTypeCovers[i];
				delete mediaTypeInfo[i];
			}
			int counter = 0;
			while (row = mysql_fetch_row(res))
			{
				for (int i = 0; i < noMovies; i++) {
					if (movies[i]->movieID == std::stoi(row[0])) {
						mediaTypeIndexes[counter] = i;
						counter++;
					}
				}
			}
			noResults = counter;
		}
		//If the query runs successfully but doesn't return any results display a message to the user
		else {
			wxMessageBox("Hmmmm...It appears there are no " + mediaType + " that fit your filters", wxT("No Results, Unfortunately"));
		}

		int currentMovie = 0;

		int value2 = 0;
		for (int i = 1; i < noResults; i++) {
			value2 = mediaTypeIndexes[i];
			while (i > 0 && value2 < mediaTypeIndexes[i - 1]) {
				mediaTypeIndexes[i] = mediaTypeIndexes[i - 1];
				i--;
			}
			mediaTypeIndexes[i] = value2;
		}

		for (int i = 0; i < noResults; i++) {
			currentMovie = mediaTypeIndexes[i];

			wxBitmap cover = *movies[currentMovie]->cover;
			//Define default cover dimensions
			cover = scaleImage(cover, coverX, coverY); //Scale the cover to the correct dimensions
			//Add each cover as a button, with an ID that is equal to the movie's ID + an int (To avoid interference with other buttons and allow for the movieID to be calculated using the ID of the object)
			mediaTypeCovers[i] = new wxBitmapButton(movieFiltering, movies[currentMovie]->movieID + 6, cover, wxPoint(((i * (coverX + (10 * frameXRatio))) - ((floor(i / 10)) * 10 * (coverX + (10 * frameXRatio))) + 15 * frameXRatio), (((floor(i / 10)) * (coverY + 100 * frameYRatio)) + (200) * frameYRatio)), wxDefaultSize, wxBORDER_NONE);
			//Bind the imageClicked function to each button
			mediaTypeCovers[i]->Bind(wxEVT_BUTTON, &cMain::movieImageClicked, this);
			std::string movieInfoStr = movies[currentMovie]->title;
			//Determine what information to display
			if (sortedField == "Title") {
				movieInfoStr = movies[currentMovie]->title;
			}
			else if (sortedField == "Director") {
				movieInfoStr = movies[currentMovie]->director;
			}
			else if (sortedField == "Release Date") {
				movieInfoStr = std::to_string(movies[currentMovie]->releaseDate);
			}
			else if (sortedField == "Age Rating") {
				movieInfoStr = movies[currentMovie]->ageRating;
			}
			else if (sortedField == "Duration") {
				movieInfoStr = movies[currentMovie]->duration;
			}
			mediaTypeInfo[i] = new wxStaticText(movieFiltering, wxID_ANY, movieInfoStr, wxPoint(mediaTypeCovers[i]->GetPosition().x + (mediaTypeCovers[i]->GetSize().GetWidth()) / 2, (((floor(i / 10)) * (coverY + 100 * frameYRatio)) + (200 * frameYRatio + coverY))), wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
			mediaTypeInfo[i]->SetFont(wxFontInfo(20 * frameXRatio)); //Scale text slightly
			mediaTypeInfo[i]->Wrap(coverX); //Wraptext
			mediaTypeInfo[i]->SetPosition(wxPoint(mediaTypeInfo[i]->GetPosition().x - (mediaTypeInfo[i]->GetSize().GetWidth()) / 2, mediaTypeInfo[i]->GetPosition().y + (15 * frameYRatio))); //Center text under cover
		}
	}
	//If the query fails to run successfully display the error number and the meaning of the error
	else {
		wxString queryErrNo = wxString::Format(wxT(" % i"), mysql_errno(conn));
		queryErrNo.erase(0, 2);
		wxMessageBox("Failed\n\nError " + queryErrNo + ": " + mysql_error(conn), wxT("Error"));
	}
}

/* <-----Procedure that picks a random movie, or takes in a specific movieID, and displays relevant info---->*/
void cMain::runRandomQuery(std::string mediaType, bool random, int movieID)
{
	//If random movie is wanted
	if (random)
	{
		movieID = randInt(1, noMovies); //Generate random movieID
		//If the movie picked is the same as the previous one, reroll until a different one is found
		while (movieID == previousMovie) {
			movieID = randInt(1, noMovies);
		}
		previousMovie = movieID;
	}
	for (int i = 0; i < noStyles; i++) //Loop for the amount of style tags the previous movie had
	{
		delete styleList[i]; //Delete style tag buttons one by one
	}
	for (int i = 0; i < noFeatures; i++) //Loop for the amount of style tags the previous movie had
	{
		delete featureList[i]; //Delete style tag buttons one by one
	}
	int currentMovie = 0;
	for (int i = 0; i < noMovies; i++) {
		if (movies[i]->movieID == movieID) {
			currentMovie = i;
		}
	}
	movieTitle->SetLabel(movies[currentMovie]->title + " (" + std::to_string(movies[currentMovie]->releaseDate) + ") - " + std::to_string(stoi(movies[currentMovie]->duration.substr(0, 2))) + (stoi(movies[currentMovie]->duration.substr(0, 2)) == 1 ? "hr " : "hrs ") + std::to_string(stoi(movies[currentMovie]->duration.substr(3, 2))) + (stoi(movies[currentMovie]->duration.substr(3, 2)) == 1 ? "min " : "mins ")); //Set the text to the name of the movie
	movieAge->SetBitmap(scaleImage(wxBitmap("Resources/Gallery/UI/BBFC/" + movies[currentMovie]->ageRating + ".jpg", wxBITMAP_TYPE_JPEG), 75 * frameXRatio, 75 * frameYRatio));
	movieAge->SetPosition(wxPoint(movieTitle->GetPosition().x + movieTitle->GetSize().GetWidth() + 25 * frameXRatio, movieTitle->GetPosition().y - 10 * frameYRatio));
	moviePoster->SetBitmap(scaleImage(*movies[currentMovie]->cover, 500 * frameXRatio, 700 * frameYRatio)); //Load the poster
	movieDesc->SetLabel("The " + mediaType.substr(0, mediaType.size() - 1) + " directed by " + (movies[currentMovie]->director.rfind("The", 0) == 0 ? (char)(tolower(movies[currentMovie]->director[0])) + movies[currentMovie]->director.substr(1, movies[currentMovie]->director.size() - 1) : movies[currentMovie]->director) + ", follows " + movies[currentMovie]->description); //Update description
	movieDesc->Wrap(frameX - (movieDesc->GetPosition().x + 2 * (headOffset * frameXRatio))); //Wraptext
	movieRandom->Fit(); //Update window after wrapping text

	/* <-----Add style buttons----->*/

	int styleCounter = 0; //Initialise/reset counter
	noStyles = 8;
	for (int i = 0; i < 8; i++) {
		if (movies[currentMovie]->styles[i] == "") {
			noStyles--;
		}
	}
	for (int i = 0; i < noStyles; i++) //
	{
		//If this is the first button to be added:
		if (styleCounter == 0)
		{
			styleList[styleCounter] = new wxButton(movieRandom, styleCounter + (noMovies + 1), movies[currentMovie]->styles[i], wxPoint(movieTitle->GetPosition().x + movieTitle->GetSize().GetWidth() + ((frameX - (headOffset * frameXRatio) - movieTitle->GetSize().GetWidth()) / 2), offsetY * frameYRatio)); //Add button basing position off of size and pos of the title of the film
		}
		else
		{
			styleList[styleCounter] = new wxButton(movieRandom, styleCounter + (noMovies + 1), movies[currentMovie]->styles[i], wxPoint((styleList[styleCounter - 1]->GetPosition().x + styleList[styleCounter - 1]->GetSize().GetWidth()), offsetY * frameYRatio)); //Add button basing position off of size and pos of the previous button added
		}
		styleList[styleCounter]->Bind(wxEVT_BUTTON, &cMain::movieStyleClicked, this); //Bind styleClicked function to added button
		styleCounter++;
	}
	int totalBtnSize = 0;
	//Works out the total width of all buttons combined
	for (int i = 0; i < styleCounter; i++) {
		totalBtnSize = totalBtnSize + styleList[i]->GetSize().GetWidth();
	}
	//Shifts each button according to the space the buttons occupy
	for (int i = 0; i < styleCounter; i++) {
		styleList[i]->Move(wxPoint(styleList[i]->GetPosition().x - (totalBtnSize) / 2, styleList[i]->GetPosition().y));
	}

	/* <-----Add feature buttons----->*/

	int featureCounter = 0; //Initialise/reset counter
	noFeatures = 4;
	for (int i = 0; i < 4; i++) {
		if (movies[currentMovie]->features[i] == "") {
			noFeatures--;
		}
	}
	for (int i = 0; i < noFeatures; i++) //
	{
		//If this is the first button to be added:
		if (featureCounter == 0)
		{
			featureList[featureCounter] = new wxButton(movieRandom, featureCounter + (noMovies + 1), movies[currentMovie]->features[i], wxPoint(movieTitle->GetPosition().x + movieTitle->GetSize().GetWidth() + ((frameX - (headOffset * frameXRatio) - movieTitle->GetSize().GetWidth()) / 2), offsetY * frameYRatio + styleList[0]->GetSize().GetHeight())); //Add button basing position off of size and pos of the title of the film
		}
		else
		{
			featureList[featureCounter] = new wxButton(movieRandom, featureCounter + (noMovies + 1), movies[currentMovie]->features[i], wxPoint((featureList[featureCounter - 1]->GetPosition().x + featureList[featureCounter - 1]->GetSize().GetWidth()), offsetY * frameYRatio + styleList[0]->GetSize().GetHeight())); //Add button basing position off of size and pos of the previous button added
		}
		featureList[featureCounter]->Bind(wxEVT_BUTTON, &cMain::movieFeatureClicked, this); //Bind featureClicked function to added button
		featureCounter++;
	}
	totalBtnSize = 0;
	//Works out the total width of all buttons combined
	for (int i = 0; i < featureCounter; i++) {
		totalBtnSize = totalBtnSize + featureList[i]->GetSize().GetWidth();
	}
	//Shifts each button according to the space the buttons occupy
	for (int i = 0; i < featureCounter; i++) {
		featureList[i]->Move(wxPoint(featureList[i]->GetPosition().x - (totalBtnSize) / 2, featureList[i]->GetPosition().y));
	}
}

/* <------------------Main ------------------>*/

/* <-----Main Procedure----->*/
cMain::cMain() : wxFrame(nullptr, wxID_ANY, "RECOMMENGINE", wxPoint(0, 0))
{
	//Maximise the app
	cMain::Maximize(true);

	//Get frame size for scaling
	frameX = cMain::GetSize().GetWidth();
	frameY = cMain::GetSize().GetHeight();

	//Calculate ratio (scaling multiplier)
	frameXRatio = (frameX - 40) / (2560.0 - 40.0); //Take 40 from both values as that's the fixed width of the scrollbar
	frameYRatio = frameY / 1400.0; //40 is taken off of my screen resolution (1440) here since the taskbar always takes up 40px

	if (frameX != 2560) { //If the user has a different screen size
		//Adjust default cover dimensions to account for the difference in screen size
		float tempCoverX = round(coverX * frameXRatio);
		float tempCoverY = round(coverY * frameYRatio);
		coverX = tempCoverX;
		coverY = tempCoverY;
	}

	//Define fonts
	wxFont titleFont(wxFontInfo(titleFontSize * frameXRatio).Bold());
	wxFont NBfont(wxFontInfo(NBfontSize * frameXRatio).Bold());
	wxFont headingFont(wxFontInfo(floor(headingFontSize * frameXRatio)));
	wxFont btnFont(wxFontInfo(btnFontSize * frameXRatio));

	//Establish containers
	basePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, cMain::GetSize());
	title = new wxStaticText(basePanel, wxID_ANY, "RECOMMENGINE", wxDefaultPosition, wxSize(wxWindow::GetSize().GetWidth(), 0), wxALIGN_CENTER);
	title->SetFont(titleFont);
	title->SetBackgroundColour("Grey");
	searchTypeNB = new wxNotebook(basePanel, wxID_ANY, wxPoint(0, title->GetSize().GetHeight()), basePanel->GetSize());
	movieFiltering = new wxScrolledWindow(searchTypeNB, wxID_ANY);
	movieFiltering->SetScrollbars(0, 20, 0, 127 * frameYRatio);
	movieRandom = new wxPanel(searchTypeNB, wxID_ANY);

	//Define dropdown options
	wxString movieStyles[23] = { "ANY", "Action", "Adventure", "Animation", "Arthouse", "Comedy", "Crime", "Drama", "Fantasy", "Fiction", "Historical", "Horror", "Mystery", "Noir", "Non-Fiction", "Post-Apocalyptic", "Realistic", "Romance", "Satire", "Sci-Fi", "Surreal", "Thriller", "Western" };
	wxString movieFeatures[7] = { "ANY", "Acting", "Fun", "Soundtrack", "Story", "Thought-Provoking", "Visuals" };
	wxString movieAges[6] = { "ANY", "U", "PG", "12", "15", "18" };
	wxString years[54] = { "ANY" };
	int startYear = 1969;
	for (int i = 1; i < 54; i++) {
		startYear++;
		years[i] = std::to_string(startYear);
	}
	wxString durations[50] = { "ANY" };
	int hour = 0;
	int mins = 0;
	for (int i = 1; i < 50; i++) {
		if (std::to_string(hour).length() == 1) {
			durations[i] = "0" + std::to_string(hour) + ":";
		}
		else {
			durations[i] = std::to_string(hour) + ":";
		}
		if (std::to_string(mins).length() == 1) {
			durations[i].append("0" + std::to_string(mins));
		}
		else {
			durations[i].append(std::to_string(mins));
		}

		if (mins == 45) {
			hour++;
			mins = 0;
		}
		else {
			mins = mins + 15;
		}
	}
	wxString columns[5] = { "Title", "Director", "Release Date", "Age Rating", "Duration" };

	//Add UI elements
	//Filtering
	movieFiltering->SetFont(headingFont);
	movieStyleHead = new wxStaticText(movieFiltering, wxID_ANY, "Style:", wxPoint(headOffset * frameXRatio, offsetY * frameYRatio));
	movieStyleCombo = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieStyleHead->GetPosition().x + movieStyleHead->GetSize().GetWidth() + choiceOffset * frameXRatio, (offsetY * frameYRatio) - 1), wxDefaultSize, wxArrayString(23, movieStyles));
	movieStyleCombo->SetFont(btnFont);
	movieFeatureHead = new wxStaticText(movieFiltering, wxID_ANY, "Outstanding Element:", wxPoint(movieStyleCombo->GetPosition().x + movieStyleCombo->GetSize().GetWidth() + headOffset * frameXRatio, offsetY * frameYRatio));
	movieFeatureCombo = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieFeatureHead->GetPosition().x + movieFeatureHead->GetSize().GetWidth() + choiceOffset * frameXRatio, (offsetY * frameYRatio) - 1), wxDefaultSize, wxArrayString(7, movieFeatures));
	movieFeatureCombo->SetFont(btnFont);
	movieAgeHead = new wxStaticText(movieFiltering, wxID_ANY, "Age Rating:", wxPoint(movieFeatureCombo->GetPosition().x + movieFeatureCombo->GetSize().GetWidth() + headOffset * frameXRatio, offsetY * frameYRatio));
	movieAgeCombo = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieAgeHead->GetPosition().x + movieAgeHead->GetSize().GetWidth() + choiceOffset * frameXRatio, (offsetY * frameYRatio) - 1), wxDefaultSize, wxArrayString(6, movieAges));
	movieAgeCombo->SetFont(btnFont);
	movieReleaseHead = new wxStaticText(movieFiltering, wxID_ANY, "Release Date Range:", wxPoint(movieAgeCombo->GetPosition().x + movieAgeCombo->GetSize().GetWidth() + headOffset * frameXRatio, offsetY * frameYRatio));
	movieReleaseStartVal = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieReleaseHead->GetPosition().x + movieReleaseHead->GetSize().GetWidth() + choiceOffset * frameXRatio, (offsetY * frameYRatio) - 1), wxDefaultSize, wxArrayString(54, years));
	movieReleaseStartVal->SetFont(btnFont);
	movieReleaseStartVal->SetStringSelection("ANY");
	movieReleaseEndVal = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieReleaseStartVal->GetPosition().x + movieReleaseStartVal->GetSize().GetWidth() + choiceOffset * frameXRatio, (offsetY * frameYRatio) - 1), wxDefaultSize, wxArrayString(54, years));
	movieReleaseEndVal->SetFont(btnFont);
	movieReleaseEndVal->SetStringSelection("ANY");
	movieDurationHead = new wxStaticText(movieFiltering, wxID_ANY, "Duration Range:", wxPoint(movieReleaseEndVal->GetPosition().x + movieReleaseEndVal->GetSize().GetWidth() + headOffset * frameXRatio, offsetY * frameYRatio));
	movieDurationStartVal = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieDurationHead->GetPosition().x + movieDurationHead->GetSize().GetWidth() + choiceOffset * frameXRatio, (offsetY * frameYRatio) - 1), wxDefaultSize, wxArrayString(50, durations));
	movieDurationStartVal->SetFont(btnFont);
	movieDurationStartVal->SetStringSelection("ANY");
	movieDurationEndVal = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieDurationStartVal->GetPosition().x + movieDurationStartVal->GetSize().GetWidth() + choiceOffset * frameXRatio, (offsetY * frameYRatio) - 1), wxDefaultSize, wxArrayString(50, durations));
	movieDurationEndVal->SetFont(btnFont);
	movieDurationEndVal->SetStringSelection("ANY");
	movieApplyFiltersBtn = new wxButton(movieFiltering, 1, "APPLY", wxPoint(movieDurationEndVal->GetPosition().x + movieDurationEndVal->GetSize().GetWidth() + (8.0 * frameXRatio * headOffset) * frameXRatio, (offsetY * frameYRatio) - 1));
	movieApplyFiltersBtn->SetFont(btnFont);
	movieResetFiltersBtn = new wxButton(movieFiltering, 2, "RESET", wxPoint(movieApplyFiltersBtn->GetPosition().x + movieApplyFiltersBtn->GetSize().GetWidth() + headOffset * frameXRatio, (offsetY * frameYRatio) - 1));
	movieResetFiltersBtn->SetFont(btnFont);
	movieSortDirection = new wxBitmapButton(movieFiltering, 4, scaleImage(wxBitmap("Resources/Gallery/UI/Sorting/Alpha - Ascending.png", wxBITMAP_TYPE_PNG), (int)(34 * frameXRatio + 0.5), (int)(34 * frameYRatio + 0.5)), wxPoint(headOffset * frameXRatio, offsetY * frameYRatio + (60 * frameYRatio) - 1));
	movieSortChoice = new wxChoice(movieFiltering, 5, wxPoint(movieSortDirection->GetPosition().x + movieSortDirection->GetSize().GetWidth() + (1.0 / 2.0) * choiceOffset * frameXRatio, (offsetY * frameYRatio) + 60 * frameYRatio), wxDefaultSize, wxArrayString(5, columns));
	movieSortChoice->SetSelection(0);
	//Random
	movieRandom->SetFont(headingFont);
	movieTitle = new wxStaticText(movieRandom, wxID_ANY, "Blade Runner (1982) - 1hr 57 mins", wxPoint(30 * frameXRatio, 30 * frameYRatio));
	movieTitle->SetFont(titleFont);
	movieAge = new wxStaticBitmap(movieRandom, wxID_ANY, scaleImage(wxBitmap("Resources/Gallery/UI/BBFC/15.jpg", wxBITMAP_TYPE_JPEG), 75 * frameXRatio, 75 * frameYRatio), wxPoint(movieTitle->GetPosition().x + movieTitle->GetSize().GetWidth() + 25 * frameXRatio, movieTitle->GetPosition().y - 10 * frameYRatio));
	moviePoster = new wxStaticBitmap(movieRandom, wxID_ANY, scaleImage(wxBitmap("Resources/Gallery/movies/Blade Runner/Cover.png", wxBITMAP_TYPE_PNG), 500 * frameXRatio, 700 * frameYRatio), wxPoint(10 * frameXRatio, 100 * frameYRatio));
	movieDescHeader = new wxStaticText(movieRandom, wxID_ANY, "SUMMARY:", wxPoint(550 * frameXRatio, 120 * frameYRatio));
	movieDesc = new wxStaticText(movieRandom, wxID_ANY, "In the film directed by Ridley Scott, Rick Deckard, an ex - policeman, becomes a special agent with a mission to exterminate a group of violent androids.\nAs he starts getting deeper into his mission, he questions his own identity.", wxPoint(550 * frameXRatio, 200 * frameYRatio));
	randomizerButton = new wxButton(movieRandom, 3, "NEW RANDOM MOVIE", wxPoint(100 * frameXRatio, 850 * frameYRatio));
	moviePoster->SetFocus();

	//Attach containers to pages of notebook and apply font
	searchTypeNB->AddPage(movieFiltering, "FILTER");
	searchTypeNB->AddPage(movieRandom, "RANDOM");
	searchTypeNB->SetFont(NBfont);

	//Establish SQL connection
	conn = mysql_init(0);
	conn = mysql_real_connect(conn, "localhost", "root", "", "ah_recommengine_db", 3306, NULL, 0);
	movieStyleCombo->SetSelection(0);
	movieFeatureCombo->SetSelection(0);
	movieAgeCombo->SetSelection(0);
	noMovies = getNumberOfEntries("movies");
	for (int i = 0; i < noMovies; i++) {
		initializeMovies(i);
	}
	runFilterQuery("movies", movieCovers, movieInfo);
	runRandomQuery("movies");
}

cMain::~cMain()
{

}