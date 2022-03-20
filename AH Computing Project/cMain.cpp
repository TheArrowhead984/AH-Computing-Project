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
float offsetX = 0.01;
float offsetY = 0.035;
int titleFontSize = 30;
int NBfontSize = 20;
int headingFontSize = 20;
int btnFontSize = 18;
int frameX = 0;
int frameY = 0;
double frameXRatio = 0.0;
double frameYRatio = 0.0;

//Sorting Variables
std::string movieSortDataType = "Alpha";
bool movieSort = false; //boolean for which direction to sort (ascending (false) or descending (true))
bool movieSortType = true; //boolean for what data type is being sorted (int (false) or string (true))

//Result Variables
int noMovies = 0;
int noResults = 0;
int noStyles = 0;
wxBitmapButton *movieCovers[100];
wxStaticText *movieInfo[100];
wxButton *styleList[8];
int previousMovie = 0;
movie *movies[100];

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
		else { return 44; }
	}
	else { return 44; }
}

void initializeMovies(int movieID) {

	qstate = mysql_query(conn, ("SELECT * FROM movies WHERE movieID = " + std::to_string(movieID)).c_str()); //Run query
	if (!qstate) //If query executes without error
	{
		res = mysql_store_result(conn);
		if (res->row_count > 0) //If the query returns results
		{
			while (row = mysql_fetch_row(res)) {
				std::string testArr[8];
				std::stringstream iss(row[7]); //Prepare string for splitting (Assign stringstream data type)
				int counter = 0; //Initialise/reset counter
				while (iss.good()) //While EOF not hit
				{
					std::string singleLine;
					getline(iss, singleLine, '/'); //Split into seperate styles ('/' is delimiter)
					testArr[counter] = singleLine;
					counter++;
				}
				movies[movieID] = new movie(new wxBitmap("Gallery/movies/" + std::string(row[1]) + "/Cover.png", wxBITMAP_TYPE_PNG), std::string(row[1]), std::string(row[2]), std::stoi(row[3]), std::string(row[4]), std::string(row[5]), testArr, std::string(row[8]));
			}
		}
	}
}

/* <------------------Event Procedures------------------>*/

/* <-----Procedure that translates selections from choice widgets to vars----->*/
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
	movieSortDirection->SetBitmap(scaleImage(wxBitmap("Gallery/UI/" + movieSortDataType + " - Ascending.png", wxBITMAP_TYPE_PNG), 34, 34));
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
	movieSortDirection->SetBitmap(scaleImage(wxBitmap("Gallery/UI/" + movieSortDataType + " - Ascending.png", wxBITMAP_TYPE_PNG), 34, 34));
	runFilterQuery("movies", movieCovers, movieInfo, true, false, false, false, false, btn->GetLabel()); //Filter for the style the user selected
}

/* <-----Procedure that generates a random movie----->*/
void cMain::newRandomMovie(wxCommandEvent &evt)
{
	runRandomQuery("movies", true, 0);
}

void cMain::changeSortDirection(wxCommandEvent &evt)
{
	if (movieSort) {
		movieSortDirection->SetBitmap(scaleImage(wxBitmap("Gallery/UI/" + movieSortDataType + " - Ascending.png", wxBITMAP_TYPE_PNG), 34, 34));
		movieSort = false;
	}
	else {
		movieSortDirection->SetBitmap(scaleImage(wxBitmap("Gallery/UI/" + movieSortDataType + " - Descending.png", wxBITMAP_TYPE_PNG), 34, 34));
		movieSort = true;
	}
}

void cMain::sortChoiceMade(wxCommandEvent &evt)
{
	if (movieSortChoice->GetStringSelection() == "Title" || movieSortChoice->GetStringSelection() == "Director") {
		movieSortDataType = "Alpha";
	}
	else {
		movieSortDataType = "Number";
	}
	if (movieSort) {
		movieSortDirection->SetBitmap(scaleImage(wxBitmap("Gallery/UI/" + movieSortDataType + " - Descending.png", wxBITMAP_TYPE_PNG), 34, 34));
	}
	else {
		movieSortDirection->SetBitmap(scaleImage(wxBitmap("Gallery/UI/" + movieSortDataType + " - Ascending.png", wxBITMAP_TYPE_PNG), 34, 34));
	}
}

/* <-----Procedure that constructs and executes SQL queries from selected filters----->*/
void cMain::runFilterQuery(std::string mediaType, wxBitmapButton *mediaTypeCovers[100], wxStaticText *mediaTypeInfo[100], bool filterStyle, bool filterFeature, bool filterAge, bool filterRelease, bool filterDuration, wxString selectedStyle, wxString selectedFeature, wxString selectedAge, wxString selectedReleaseStart, wxString selectedReleaseEnd, wxString selectedDurationStart, wxString selectedDurationEnd, wxString sortedField, bool sortDirection)
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
	sortedField.erase(std::remove(sortedField.begin(), sortedField.end(), ' '), sortedField.end());
	queryStr.append(" ORDER BY " + sortedField + " " + (sortDirection ? "desc" : "asc"));
	//Execute constructed query
	qstate = mysql_query(conn, queryStr.c_str());
	if (!qstate) //If query executes successfully
	{
		res = mysql_store_result(conn);
		int counter = 0; //Initialise counter var

		if (res->row_count > 0) {
			//Delete all covers currently on display from previous query
			for (int i = 0; i < noResults; i++) {
				delete mediaTypeCovers[i];
				delete mediaTypeInfo[i];
			}
			while (row = mysql_fetch_row(res))
			{
				wxBitmap cover = *movies[std::stoi(row[0])]->cover;
				//Define default cover dimensions
				int coverX = 237;
				int coverY = 355;
				if (frameX != 2560) { //If the user has a different screen size
					//Adjust default cover dimensions to account for the difference in screen size
					float tempCoverX = round(coverX * frameXRatio);
					float tempCoverY = round(coverY * frameYRatio);
					coverX = tempCoverX;
					coverY = tempCoverY;
				}
				cover = scaleImage(cover, coverX, coverY); //Scale the cover to the correct dimensions
				//Add each cover as a button, with an ID that is equal to the movie's ID + an int (To avoid interference with other buttons and allow for the movieID to be calculated using the ID of the object)
				mediaTypeCovers[counter] = new wxBitmapButton(movieFiltering, std::stoi(row[0]) + 6, cover, wxPoint((counter * (coverX + 10)) - ((floor(counter / 10)) * 10 * (coverX + 10)) + 20, (((floor(counter / 10)) * (coverY + 100)) + (200) * frameYRatio)), wxDefaultSize, wxBORDER_NONE);
				//Bind the imageClicked function to each button
				mediaTypeCovers[counter]->Bind(wxEVT_BUTTON, &cMain::movieImageClicked, this);
				std::string movieInfoStr = movies[std::stoi(row[0])]->title;
				//Determine what information to display
				if (sortedField == "Title") {
					movieInfoStr = movies[std::stoi(row[0])]->title;
				}
				else if (sortedField == "Director") {
					movieInfoStr = movies[std::stoi(row[0])]->director;
				}
				else if (sortedField == "ReleaseDate") {
					movieInfoStr = std::to_string(movies[std::stoi(row[0])]->releaseDate);
				}
				else if (sortedField == "AgeRating") {
					movieInfoStr = movies[std::stoi(row[0])]->ageRating;
				}
				else if (sortedField == "Duration") {
					movieInfoStr = movies[std::stoi(row[0])]->duration;
				}
				mediaTypeInfo[counter] = new wxStaticText(movieFiltering, wxID_ANY, movieInfoStr, wxPoint(mediaTypeCovers[counter]->GetPosition().x + (mediaTypeCovers[counter]->GetSize().GetWidth()) / 2, (((floor(counter / 10)) * (coverY + 100)) + (200 + coverY) * frameYRatio)), wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
				mediaTypeInfo[counter]->SetFont(wxFontInfo(18 * frameXRatio).Light()); //Scale text slightly
				mediaTypeInfo[counter]->Wrap(coverX); //Wraptext
				mediaTypeInfo[counter]->SetPosition(wxPoint(mediaTypeInfo[counter]->GetPosition().x - (mediaTypeInfo[counter]->GetSize().GetWidth()) / 2, mediaTypeInfo[counter]->GetPosition().y + (15 * frameYRatio))); //Center text under cover
				counter++;
				noResults = counter;
			}
		}
		//If the query runs successfully but doesn't return any results display a message to the user
		else {
			wxMessageBox("Hmmmm...It appears there are no " + mediaType + " that fit your filters", wxT("No Results, Unfortunately"));
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
	wxString queryStr = "";
	//If random movie is wanted
	if (random == true)
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
	movieTitle->SetLabel(movies[movieID]->title + " (" + std::to_string(movies[movieID]->releaseDate) + ")"); //Set the text to the name of the movie
	moviePoster->SetBitmap(scaleImage(*movies[movieID]->cover, 500, 700)); //Load the poster
	movieDesc->SetLabel("The " + mediaType.substr(0, mediaType.size() - 1) + " directed by " + movies[movieID]->director + ", follows " + movies[movieID]->description); //Update description
	movieDesc->Wrap(frameX - (movieDesc->GetPosition().x + (offsetX * frameX))); //Wraptext
	movieRandom->Fit(); //Update window after wrapping text
	std::stringstream iss(); //Prepare string for splitting (Assign stringstream data type)
	int counter = 0; //Initialise/reset counter
	noStyles = 8;
	for (int i = 0; i < 8; i++) {
		if (movies[movieID]->styles[i] == "") {
			noStyles--;
		}
	}
	for (int i = 0; i < noStyles; i++) //
	{
		//If this is the first button to be added:
		if (counter == 0)
		{
			styleList[counter] = new wxButton(movieRandom, counter + (noMovies + 1), movies[movieID]->styles[i], wxPoint(movieTitle->GetPosition().x + movieTitle->GetSize().GetWidth() + ((frameX - (offsetX * frameX) - movieTitle->GetSize().GetWidth()) / 2), 50)); //Add button basing position off of size and pos of the title of the film
		}
		else
		{
			styleList[counter] = new wxButton(movieRandom, counter + (noMovies + 1), movies[movieID]->styles[i], wxPoint((styleList[counter - 1]->GetPosition().x + styleList[counter - 1]->GetSize().GetWidth()), 50)); //Add button basing position off of size and pos of the previous button added
		}
		styleList[counter]->Bind(wxEVT_BUTTON, &cMain::movieStyleClicked, this); //Bind styleClicked function to added button
		counter++;
		//noStyles = counter;
	}
	int totalBtnSize = 0;
	//Works out the total width of all buttons combined
	for (int i = 0; i < counter; i++) {
		totalBtnSize = totalBtnSize + styleList[i]->GetSize().GetWidth();
	}
	//Shifts each button according to the space the buttons occupy
	for (int i = 0; i < counter; i++) {
		styleList[i]->Move(wxPoint(styleList[i]->GetPosition().x - (totalBtnSize) / 2, styleList[i]->GetPosition().y));
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
	frameXRatio = frameX / 2560.0;
	frameYRatio = frameY / 1400.0;

	//Define fonts
	wxFont titleFont(wxFontInfo(titleFontSize * frameXRatio).Bold());
	wxFont NBfont(wxFontInfo(NBfontSize * frameXRatio).Bold());
	wxFont headingFont(wxFontInfo(headingFontSize * frameXRatio).Light());
	wxFont btnFont(wxFontInfo(btnFontSize * frameXRatio).Light());

	//Establish containers
	basePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, cMain::GetSize());
	title = new wxStaticText(basePanel, wxID_ANY, "RECOMMENGINE", wxDefaultPosition, wxSize(wxWindow::GetSize().GetWidth(), 0), wxALIGN_CENTER);
	title->SetFont(titleFont);
	title->SetBackgroundColour("Grey");
	mediaNB = new wxNotebook(basePanel, wxID_ANY, wxPoint(0, title->GetSize().GetHeight()), basePanel->GetSize(), wxNB_LEFT);
	searchTypeNB = new wxNotebook(mediaNB, wxID_ANY, wxPoint(offsetX * frameX, 0));
	movieFiltering = new wxScrolledWindow(searchTypeNB, wxID_ANY);
	movieFiltering->SetScrollbars(0, 20, 0, (125 * frameYRatio));
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
	movieStyleHead = new wxStaticText(movieFiltering, wxID_ANY, "Style:", wxPoint(offsetX * frameX, offsetY * frameY));
	movieStyleCombo = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieStyleHead->GetPosition().x + movieStyleHead->GetSize().GetWidth() + (headingFontSize * offsetX) / 100 * frameX, (offsetY * frameY) - 1), wxDefaultSize, wxArrayString(23, movieStyles));
	movieStyleCombo->SetFont(btnFont);
	movieFeatureHead = new wxStaticText(movieFiltering, wxID_ANY, "Outstanding Element:", wxPoint(movieStyleCombo->GetPosition().x + movieStyleCombo->GetSize().GetWidth() + offsetX * frameX, offsetY * frameY));
	movieFeatureCombo = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieFeatureHead->GetPosition().x + movieFeatureHead->GetSize().GetWidth() + (headingFontSize * offsetX) / 100 * frameX, (offsetY * frameY) - 1), wxDefaultSize, wxArrayString(7, movieFeatures));
	movieFeatureCombo->SetFont(btnFont);
	movieAgeHead = new wxStaticText(movieFiltering, wxID_ANY, "Age Rating:", wxPoint(movieFeatureCombo->GetPosition().x + movieFeatureCombo->GetSize().GetWidth() + offsetX * frameX, offsetY * frameY));
	movieAgeCombo = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieAgeHead->GetPosition().x + movieAgeHead->GetSize().GetWidth() + (headingFontSize * offsetX) / 100 * frameX, (offsetY * frameY) - 1), wxDefaultSize, wxArrayString(6, movieAges));
	movieAgeCombo->SetFont(btnFont);
	movieReleaseHead = new wxStaticText(movieFiltering, wxID_ANY, "Release Date Range:", wxPoint(movieAgeCombo->GetPosition().x + movieAgeCombo->GetSize().GetWidth() + offsetX * frameX, offsetY * frameY));
	movieReleaseStartVal = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieReleaseHead->GetPosition().x + movieReleaseHead->GetSize().GetWidth() + offsetX * frameX, (offsetY * frameY) - 1), wxDefaultSize, wxArrayString(54, years));
	movieReleaseStartVal->SetFont(btnFont);
	movieReleaseStartVal->SetStringSelection("ANY");
	movieReleaseEndVal = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieReleaseStartVal->GetPosition().x + movieReleaseStartVal->GetSize().GetWidth() + offsetX * frameX, (offsetY * frameY) - 1), wxDefaultSize, wxArrayString(54, years));
	movieReleaseEndVal->SetFont(btnFont);
	movieReleaseEndVal->SetStringSelection("ANY");
	movieDurationHead = new wxStaticText(movieFiltering, wxID_ANY, "Duration Range:", wxPoint(movieReleaseEndVal->GetPosition().x + movieReleaseEndVal->GetSize().GetWidth() + offsetX * frameX, offsetY * frameY));
	movieDurationStartVal = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieDurationHead->GetPosition().x + movieDurationHead->GetSize().GetWidth() + offsetX * frameX, (offsetY * frameY) - 1), wxDefaultSize, wxArrayString(50, durations));
	movieDurationStartVal->SetFont(btnFont);
	movieDurationStartVal->SetStringSelection("ANY");
	movieDurationEndVal = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieDurationStartVal->GetPosition().x + movieDurationStartVal->GetSize().GetWidth() + offsetX * frameX, (offsetY * frameY) - 1), wxDefaultSize, wxArrayString(50, durations));
	movieDurationEndVal->SetFont(btnFont);
	movieDurationEndVal->SetStringSelection("ANY");
	movieApplyFiltersBtn = new wxButton(movieFiltering, 1, "APPLY", wxPoint(movieDurationEndVal->GetPosition().x + movieDurationEndVal->GetSize().GetWidth() + (4 * offsetX) * frameX, (offsetY * frameY) - 1));
	movieApplyFiltersBtn->SetFont(btnFont);
	movieResetFiltersBtn = new wxButton(movieFiltering, 2, "RESET", wxPoint(movieApplyFiltersBtn->GetPosition().x + movieApplyFiltersBtn->GetSize().GetWidth() + offsetX * frameX, (offsetY * frameY) - 1));
	movieResetFiltersBtn->SetFont(btnFont);
	movieSortDirection = new wxBitmapButton(movieFiltering, 4, scaleImage(wxBitmap("Gallery/UI/Alpha - Ascending.png", wxBITMAP_TYPE_PNG), 34, 34), wxPoint(offsetX * frameX, offsetY * frameY + 59));
	movieSortChoice = new wxChoice(movieFiltering, 5, wxPoint(movieSortDirection->GetPosition().x + movieSortDirection->GetSize().GetWidth() + 1 / 2 * offsetX * frameX, (offsetY * frameY) + 60), wxDefaultSize, wxArrayString(5, columns));
	movieSortChoice->SetSelection(0);
	//Random
	movieRandom->SetFont(headingFont);
	movieTitle = new wxStaticText(movieRandom, wxID_ANY, "Blade Runner (1982)", wxPoint(30, 30));
	movieTitle->SetFont(titleFont);
	moviePoster = new wxStaticBitmap(movieRandom, wxID_ANY, scaleImage(wxBitmap("Gallery/movies/Blade Runner/Cover.png", wxBITMAP_TYPE_PNG), 500, 700), wxPoint(10, 100));
	movieDescHeader = new wxStaticText(movieRandom, wxID_ANY, "SUMMARY:", wxPoint(550, 120));
	movieDesc = new wxStaticText(movieRandom, wxID_ANY, "In the film directed by Ridley Scott, Rick Deckard, an ex - policeman, becomes a special agent with a mission to exterminate a group of violent androids.\nAs he starts getting deeper into his mission, he questions his own identity.", wxPoint(550, 200));
	randomizerButton = new wxButton(movieRandom, 3, "NEW RANDOM MOVIE", wxPoint(100, 850));
	moviePoster->SetFocus();
	//Attach containers to pages of notebook and apply font
	searchTypeNB->AddPage(movieFiltering, "FILTER");
	searchTypeNB->AddPage(movieRandom, "RANDOM");
	searchTypeNB->SetFont(NBfont);
	mediaNB->AddPage(searchTypeNB, "MOVIES", true);
	mediaNB->AddPage(new wxPanel(mediaNB, 2), "TV SHOWS");
	mediaNB->AddPage(new wxPanel(mediaNB, 3), "GAMES");
	mediaNB->SetFont(NBfont);

	//Establish SQL connection
	conn = mysql_init(0);
	conn = mysql_real_connect(conn, "localhost", "root", "", "ah_recommengine_db", 3306, NULL, 0);
	movieStyleCombo->SetSelection(0);
	movieFeatureCombo->SetSelection(0);
	movieAgeCombo->SetSelection(0);
	noMovies = getNumberOfEntries("movies");
	for (int i = 1; i <= noMovies; i++) {
		initializeMovies(i);
	}
	runFilterQuery("movies", movieCovers, movieInfo);
	runRandomQuery("movies");
}

cMain::~cMain()
{

}