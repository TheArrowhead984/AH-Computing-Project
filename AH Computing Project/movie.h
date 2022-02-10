#pragma once
#include "wx/wx.h"
#include "wx/grid.h"
#include "wx/msgdlg.h"
#include "wx/notebook.h"
#include <iostream>
#include <string>

using namespace std;

class movie {
public:
    /* <-----Class Variables----->*/
    wxBitmapButton *cover = nullptr;
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
        
    }
    //Constructor
    movie(wxBitmapButton *cvr, string tle, string dir, int relDte, string ageRat, string dur, string stys[], string desc) {
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