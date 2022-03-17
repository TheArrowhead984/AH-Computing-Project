class movie {
public:
    /* <-----Class Variables----->*/
    wxBitmap *cover = nullptr;
    wxString title = "";
    wxString director = "";
    int releaseDate = 0;
    wxString ageRating = "";
    wxString duration = "";
    wxString styles[8];
    wxString description = "";

    /* <-----Class Methods----->*/
    //Default Constructor
    movie() {
        cover = nullptr;
        title = "";
        director = "";
        releaseDate = 0;
        ageRating = "";
        duration = "";
        for (int i = 0; i < 8; i++) {
            styles[i] = "";
        }
        description = "";
    }
    //Constructor
    movie(wxBitmap *cvr, std::string tle, std::string dir, int relDte, std::string ageRat, std::string dur, std::string stys[], std::string desc) {
        cover = cvr;
        title = tle;
        director = dir;
        releaseDate = relDte;
        ageRating = ageRat;
        duration = dur;
        for (int i = 0; i < 8; i++) {
            styles[i] = stys[i];
        }
        description = desc;
    }
};