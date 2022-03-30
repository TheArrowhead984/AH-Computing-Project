class movie {
public:
    /* <-----Class Variables----->*/
    int movieID = 0;
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
    //Constructor
    movie(int id, wxBitmap *cvr, std::string tle, std::string dir, int relDte, std::string ageRat, std::string dur, std::string feats[], std::string stys[], std::string desc) {
        movieID = id;
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





