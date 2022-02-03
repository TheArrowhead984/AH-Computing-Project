#include "cApp.h"
#include "mysql.h"
#include <iostream>

wxIMPLEMENT_APP(cApp);

cApp::cApp()
{

}

cApp::~cApp()
{

}

bool cApp::OnInit()
{
	wxInitAllImageHandlers();
	m_frame1 = new cMain();
	m_frame1->Show();

	return true;
}