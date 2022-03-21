class movie {
public:
    /* <-----Class Variables----->*/
    wxBitmap *cover = nullptr;
    std::string title = "";
    std::string director = "";
    int releaseDate = 0;
    std::string ageRating = "";
    std::string duration = "";
    std::string features[4];
    std::string styles[8];
    std::string description = "";

    /* <-----Class Methods----->*/
    //Default Constructor
    movie() {
        cover = nullptr;
        title = "";
        director = "";
        releaseDate = 0;
        ageRating = "";
        duration = "";
        for (int i = 0; i < 4; i++) {
            features[i] = "";
        }
        for (int i = 0; i < 8; i++) {
            styles[i] = "";
        }
        description = "";
    }
    //Constructor
    movie(wxBitmap *cvr, std::string tle, std::string dir, int relDte, std::string ageRat, std::string dur, std::string feats[], std::string stys[], std::string desc) {
        cover = cvr;
        title = tle;
        director = dir;
        releaseDate = relDte;
        ageRating = ageRat;
        duration = dur;
        for (int i = 0; i < 4; i++) {
            features[i] = feats[i];
        }
        for (int i = 0; i < 8; i++) {
            styles[i] = stys[i];
        }
        description = desc;
    }
};