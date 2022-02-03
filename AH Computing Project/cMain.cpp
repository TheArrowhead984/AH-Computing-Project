//Library inclusions
#include "cMain.h"
#include "mysql.h"
#include <iostream>
#include "mysqld_error.h"
#include <thread>
#include <chrono>
#include <typeinfo>
#include <array>
#include <random>
#include <sstream>

//Define event table
wxBEGIN_EVENT_TABLE(cMain, wxFrame)
    EVT_BUTTON(1, applyFilters)
    EVT_BUTTON(2, resetFilters)
    EVT_BUTTON(3, newRandomMovie)
wxEND_EVENT_TABLE()



/* <------------------Define global variables------------------>*/

//Namespaces
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

//SQL stuff
int qstate;
MYSQL *conn;
MYSQL_ROW row;
MYSQL_RES *res;

//Scaling values
float offsetX = 0.02;
float offsetY = 0.035;
int titleFontSize = 30;
int NBfontSize = 20;
int headingFontSize = 20;
int btnFontSize = 18;
int frameX = 0;
int frameY = 0;
double frameXRatio = 0.0;
double frameYRatio = 0.0;

//Result Variables
int noResults = 0;
int noStyles = 0;
wxBitmapButton *covers[42];
wxButton *styleList[8];
int previousMovie = 0;



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

/* <------------------Event Procedures------------------>*/

/* <-----Procedure that translates selections from choice widgets to vars----->*/
void cMain::applyFilters(wxCommandEvent &evt)
{
    //Convert selections to strings
    wxString selectedStyle = movieStyleCombo->GetStringSelection();
    wxString selectedFeature = movieFeatureCombo->GetStringSelection();
    wxString selectedAge = movieAgeCombo->GetStringSelection();
    //Initialise booleans
    bool filterStyle = false;
    bool filterFeature = false;
    bool filterAge = false;

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
    runFilterQuery("movies", filterStyle, filterFeature, filterAge, selectedStyle, selectedFeature, selectedAge, covers); //Plug info into func
}

/* <-----Procedure that resets filtering----->*/
void cMain::resetFilters(wxCommandEvent &evt)
{
    //Return selections to default
    movieStyleCombo->SetSelection(0);
    movieFeatureCombo->SetSelection(0);
    movieAgeCombo->SetSelection(0);
    runFilterQuery("movies", false, false, false, "ANY", "ANY", "ANY", covers); //Run filter query without any filters applied
}

/* <-----Procedure that generates a random movie----->*/
void cMain::newRandomMovie(wxCommandEvent &evt)
{
    runRandomQuery("movies", true, 0);
}

/* <-----Procedure that shows info for a specific film----->*/
void cMain::imageClicked(wxCommandEvent &evt)
{
    searchTypeNB->ChangeSelection(2); //Change selected page to the random page
    runRandomQuery("movies", false, evt.GetId()-3);
}

/* <-----Procedure that filters for a style shown on the random page----->*/
void cMain::styleClicked(wxCommandEvent &evt)
{
    searchTypeNB->ChangeSelection(0);
    wxButton *btn = static_cast<wxButton*>(evt.GetEventObject()); //Get a reference to the button that triggered this event
    
    movieStyleCombo->SetSelection(movieStyleCombo->FindString(btn->GetLabel())); //Set selected style filter to the text on the button the user clicked
    movieFeatureCombo->SetSelection(0);
    movieAgeCombo->SetSelection(0);
    runFilterQuery("movies", true, false, false, btn->GetLabel(), "", "", covers); //Filter for the style the user selected
}

/* <-----Procedure that constructs and executes SQL queries from selected filters----->*/
void cMain::runFilterQuery(std::string mediaType, bool filterStyle, bool filterFeature, bool filterAge, wxString selectedStyle, wxString selectedFeature, wxString selectedAge, wxBitmapButton *covers[])
{
    //Construct SQL query from user filters
    string queryStr = "SELECT * FROM " + mediaType + " WHERE ";
    if (filterStyle == true) {
        queryStr.append("INSTR(styles, '" + selectedStyle + "') <> 0");
        if (filterFeature == true || filterAge == true) { queryStr.append(" AND "); }
    }
    if (filterFeature == true) {
        queryStr.append("INSTR(features, '" + selectedFeature + "') <> 0");
        if (filterAge == true) { queryStr.append(" AND "); }
    }
    if (filterAge == true) {
        queryStr.append("ageRating = '" + selectedAge + "'");
    }
    if (filterStyle != true && filterFeature != true && filterAge != true) {
        queryStr = "SELECT * FROM " + mediaType;
    }

    //Execute constructed query
    qstate = mysql_query(conn, queryStr.c_str());
    if (!qstate) //If query executes successfully
    {
        res = mysql_store_result(conn);
        int counter = 0; //Initialise counter var
        
        if (res->row_count > 0) {
            //Delete all covers currently on display from previous query
            for (int i = 0; i <= noResults; i++) {
                delete covers[i];
            }
            while (row = mysql_fetch_row(res))
            {
                wxBitmap cover = wxBitmap("Gallery/" + mediaType + "/" + row[1] + "/Cover.png", wxBITMAP_TYPE_PNG); //Find movie cover PNG
                //Define default cover dimensions
                int coverX = 237;
                int coverY = 355;
                if (frameX != 2560) { //If the user has a different screen size
                    //Adjust default cover dimensions to account for the difference in screen size
                    float tempCoverX = round(coverX*frameXRatio);
                    float tempCoverY = round(coverY*frameYRatio);
                    coverX = tempCoverX;
                    coverY = tempCoverY;
                }
                cover = scaleImage(cover, coverX, coverY); //Scale the cover to the correct dimensions
                //Add each cover as a button, with an ID that is equal to the movie's ID + 3 (To avoid interference with other buttons) and then bind the imageClicked function to each button
                covers[counter] = new wxBitmapButton(movieFiltering, stoi(row[0])+3, cover, wxPoint((counter * (coverX+10)) - ((floor(counter / 10)) * 10 * (coverX+10))+20, (((floor(counter / 10)) * (coverY+10)) + (125)*frameYRatio)), wxDefaultSize, wxBORDER_NONE);
                covers[counter]->Bind(wxEVT_BUTTON, &cMain::imageClicked, this);
                noResults = counter;
                counter++;
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
        movieID = randInt(1, 42); //Generate random movieID
        //If the movie picked is the same as the previous one, reroll until a different one is found
        while (movieID == previousMovie) {
            movieID = randInt(1, 42);
        }
        queryStr = "SELECT * FROM movies WHERE movieID = " + to_string(movieID); //Query using randomly generated int
    }
    //If specific movie is wanted
    else {
        queryStr = "SELECT * FROM movies WHERE movieID = " + to_string(movieID); //Query using supplied movieID
    }
    qstate = mysql_query(conn, queryStr); //Run query
    if (!qstate) //If query executes without error
    {
        res = mysql_store_result(conn);
        if (res->row_count > 0) //If the query returns results
        {
            for (int i = 0; i <= noStyles; i++) //Loop for the amount of style tags the previous movie had
            {
                delete styleList[i]; //Delete style tag buttons one by one
            }
            while (row = mysql_fetch_row(res)) {
                movieTitle->SetLabel(wxString(row[1]) + " (" + row[3] + ")"); //Set the text to the name of the movie
                moviePoster->SetBitmap(scaleImage(wxBitmap("Gallery/" + mediaType + "/" + row[1] + "/Cover.png", wxBITMAP_TYPE_PNG), 500, 700)); //Load the poster
                movieDesc->SetLabel("The " + mediaType.substr(0, mediaType.size() - 1) + " directed by " + wxString(row[2]) + ", follows " + wxString(row[8])); //Update description
                movieDesc->Wrap(frameX-(movieDesc->GetPosition().x + (offsetX*frameX))); //Wraptext
                movieRandom->Fit(); //Update window after wrapping text
                std::stringstream iss(row[7]); //Prepare string for splitting (Assign stringstream data type)
                int counter = 0; //Initialise/reset counter
                while (iss.good()) //While EOF not hit
                {
                    std::string singleLine;
                    getline(iss, singleLine, '/'); //Split into seperate styles ('/' is delimiter)
                    //If this is the first button to be added:
                    if (counter == 0)
                    {
                        styleList[counter] = new wxButton(movieRandom, counter + 43, singleLine, wxPoint(movieTitle->GetPosition().x + movieTitle->GetSize().GetWidth()+((frameX-(offsetX*frameX)-movieTitle->GetSize().GetWidth())/2), 50)); //Add button basing position off of size and pos of the title of the film
                    }
                    else
                    {
                        styleList[counter] = new wxButton(movieRandom, counter + 43, singleLine, wxPoint((styleList[counter-1]->GetPosition().x + styleList[counter-1]->GetSize().GetWidth()), 50)); //Add button basing position off of size and pos of the previous button added
                    }
                    styleList[counter]->Bind(wxEVT_BUTTON, &cMain::styleClicked, this); //Bind styleClicked function to added button
                    noStyles = counter;
                    counter++;
                }
                int totalBtnSize = 0;
                //Works out the total width of all buttons combined
                for (int i = 0; i < counter; i++) {
                    totalBtnSize = totalBtnSize + styleList[i]->GetSize().GetWidth();
                }
                //Shifts each button according to the space the buttons occupy
                for (int i = 0; i < counter; i++) {
                    styleList[i]->Move(wxPoint(styleList[i]->GetPosition().x - (totalBtnSize)/2, styleList[i]->GetPosition().y));
                }
            }
        }
    }
    //Display error no. and meaning if query can't execute correctly
    else {
        wxString queryErrNo = wxString::Format(wxT(" % i"), mysql_errno(conn));
        queryErrNo.erase(0, 2);
        wxMessageBox("Failed\n\nError " + queryErrNo + ": " + mysql_error(conn), wxT("Error"));
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
    wxFont titleFont(wxFontInfo(titleFontSize*frameXRatio).Bold());
    wxFont NBfont(wxFontInfo(NBfontSize*frameXRatio).Bold());
    wxFont headingFont(wxFontInfo(headingFontSize*frameXRatio).Light());
    wxFont btnFont(wxFontInfo(btnFontSize*frameXRatio).Light());

    //Establish containers
    basePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, cMain::GetSize());
    title = new wxStaticText(basePanel, wxID_ANY, "RECOMMENGINE", wxDefaultPosition, wxSize(wxWindow::GetSize().GetWidth(), 0), wxALIGN_CENTER);
    title->SetFont(titleFont);
    title->SetBackgroundColour("Grey");
    mediaNB = new wxNotebook(basePanel, wxID_ANY, wxPoint(0, title->GetSize().GetHeight()), basePanel->GetSize(), wxNB_LEFT);
    searchTypeNB = new wxNotebook(mediaNB, wxID_ANY, wxPoint(offsetX * frameX, 0));
    movieFiltering = new wxScrolledWindow(searchTypeNB, wxID_ANY);
    movieFiltering->SetScrollbars(0, 20, 0, (100*frameYRatio));
    movieRandom = new wxPanel(searchTypeNB, wxID_ANY);
    
    //Define dropdown options
    wxString movieStyles[24] = {"ANY", "Action", "Adventure", "Animation", "Arthouse", "Comedy", "Crime", "Drama", "Fantasy", "Fiction", "Historical", "Horror", "Mystery", "Neo-Noir", "Noir", "Non-Fiction", "Post-Apocalyptic", "Realistic", "Romance", "Satire", "Sci-Fi", "Surreal", "Thriller", "Western"};
    wxString movieFeatures[7] = {"ANY", "Acting", "Fun", "Soundtrack", "Story", "Thought-Provoking", "Visuals"};
    wxString movieAges[6] = {"ANY", "U", "PG", "12", "15", "18"};

    //Add UI elements
    //Filtering
    movieFiltering->SetFont(headingFont);
    movieStyleHead = new wxStaticText(movieFiltering, wxID_ANY, "Style:", wxPoint(offsetX * frameX, offsetY * frameY));
    movieStyleCombo = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieStyleHead->GetPosition().x + movieStyleHead->GetSize().GetWidth() + (headingFontSize * offsetX)/100 * frameX, (offsetY * frameY)-1), wxDefaultSize, wxArrayString(24, movieStyles));
    movieStyleCombo->SetFont(btnFont);
    movieFeatureHead = new wxStaticText(movieFiltering, wxID_ANY, "Outstanding Element:", wxPoint(movieStyleCombo->GetPosition().x + movieStyleCombo->GetSize().GetWidth() + offsetX * frameX, offsetY * frameY));
    movieFeatureCombo = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieFeatureHead->GetPosition().x + movieFeatureHead->GetSize().GetWidth() + (headingFontSize * offsetX)/100 * frameX, (offsetY * frameY)-1), wxDefaultSize, wxArrayString(7, movieFeatures));
    movieFeatureCombo->SetFont(btnFont);
    movieAgeHead = new wxStaticText(movieFiltering, wxID_ANY, "Age Rating:", wxPoint(movieFeatureCombo->GetPosition().x + movieFeatureCombo->GetSize().GetWidth() + offsetX * frameX, offsetY * frameY));
    movieAgeCombo = new wxChoice(movieFiltering, wxID_ANY, wxPoint(movieAgeHead->GetPosition().x + movieAgeHead->GetSize().GetWidth() + (headingFontSize * offsetX)/100 * frameX, (offsetY * frameY)-1), wxDefaultSize, wxArrayString(6, movieAges));
    movieAgeCombo->SetFont(btnFont);
    applyFiltersBtn = new wxButton(movieFiltering, 1, "APPLY", wxPoint(movieAgeCombo->GetPosition().x + movieAgeCombo->GetSize().GetWidth() + offsetX * frameX, (offsetY * frameY)-1));
    applyFiltersBtn->SetFont(btnFont);
    resetFiltersBtn = new wxButton(movieFiltering, 2, "RESET", wxPoint(applyFiltersBtn->GetPosition().x + applyFiltersBtn->GetSize().GetWidth() + offsetX * frameX, (offsetY * frameY)-1));
    resetFiltersBtn->SetFont(btnFont);
    //Random
    movieRandom->SetFont(headingFont);
    movieTitle = new wxStaticText(movieRandom, wxID_ANY, "Blade Runner (1982)", wxPoint(30, 30));
    movieTitle->SetFont(titleFont);
    moviePoster = new wxStaticBitmap(movieRandom, wxID_ANY, scaleImage(wxBitmap("Covers/movies/Blade Runner.png", wxBITMAP_TYPE_PNG), 500, 700), wxPoint(10, 100));
    movieDescHeader = new wxStaticText(movieRandom, wxID_ANY, "SUMMARY:", wxPoint(550, 120));
    movieDesc = new wxStaticText(movieRandom, wxID_ANY, "In the film directed by Ridley Scott, Rick Deckard, an ex - policeman, becomes a special agent with a mission to exterminate a group of violent androids.\nAs he starts getting deeper into his mission, he questions his own identity.", wxPoint(550, 200));
    randomizerButton = new wxButton(movieRandom, 3, "NEW RANDOM MOVIE", wxPoint(100, 850));
    moviePoster->SetFocus();

    //Attach containers to pages of notebook and apply font
    searchTypeNB->AddPage(movieFiltering, "FILTER");
    searchTypeNB->AddPage(new wxPanel(searchTypeNB, 2), "SEARCH");
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
    runFilterQuery("movies", false, false, false, "ANY", "ANY", "ANY", covers);
    runRandomQuery("movies", false, 1);
}

cMain::~cMain()
{

}