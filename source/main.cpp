#include <wx/wx.h>
#include <wx/dcsvg.h>
#include <wx/textfile.h>
#include <string>
#include <thread>
#include <pqxx/pqxx>
#include <iostream>

#include "ids.hpp"
 
class MyApp : public wxApp
{
    public:
        virtual bool OnInit();
       
};
 
class MyFrame : public wxFrame
{
public:
    MyFrame();
    ~MyFrame();
 
private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnSubmit(wxCommandEvent& event);
    void OnStartOpService(wxCommandEvent& event);
    void OnEndOpService(wxCommandEvent& event);
    void OnEndOpWorking(wxCommandEvent& event);

    pqxx::connection *db_connection;
};
 
wxIMPLEMENT_APP(MyApp);
 
bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::~MyFrame() {
}
 
MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Queue Terminal", wxDefaultPosition, wxSize(500, 500))
{

    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                     "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
 
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
 
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
 
    SetMenuBar( menuBar ); 

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    
    wxTextCtrl *customerName = new wxTextCtrl(this, ID_TEXT_CTRL);
    wxButton *creare_record_queue = new wxButton(this, ID_REGISTER, wxT("Create Record Queue"));
    wxButton *start_operator_service = new wxButton(this, ID_START_OP_SERVICE, wxT("Start Operator Service"));
    wxButton *end_operator_service = new wxButton(this, ID_END_OP_SERVICE, wxT("End Operator Service")); 
    wxButton *end_operator_working = new wxButton(this, ID_END_OP_WORKING, wxT("End Operator Working")); 

    
    vbox->Add(customerName, 1, wxEXPAND | wxALL, 20);
    vbox->Add(creare_record_queue, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
    vbox->Add(start_operator_service, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
    vbox->Add(end_operator_service, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
    vbox->Add(end_operator_working, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);

    SetSizer(vbox);
 
    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);
    Bind(wxEVT_BUTTON, &MyFrame::OnSubmit, this, ID_REGISTER, -1, customerName);   
    Bind(wxEVT_BUTTON, &MyFrame::OnStartOpService, this, ID_START_OP_SERVICE, -1, customerName);   
    Bind(wxEVT_BUTTON, &MyFrame::OnEndOpService, this, ID_END_OP_SERVICE, -1, customerName);   
    Bind(wxEVT_BUTTON, &MyFrame::OnEndOpWorking, this, ID_END_OP_WORKING, -1, customerName);   

    try {
        db_connection = new pqxx::connection("user=postgres host=localhost password=password dbname=transactions");
        if (db_connection->is_open()) {
            std::cout << "Opened database successfully: " << db_connection->dbname() << std::endl;
        } else {
            std::cout << "Can't open database" << std::endl;
        }
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    } 
}
 
void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}
 
void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
                 "About Hello World", wxOK | wxICON_INFORMATION);
}
 
void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

void MyFrame::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    dc.SetBackground( *wxWHITE_BRUSH );
    dc.Clear();
}


void MyFrame::OnSubmit(wxCommandEvent& event) {
    // std::cout << "CLICKED";

    wxTextCtrl *customerNameField = static_cast<wxTextCtrl *>(event.GetEventUserData());
    std::cout << customerNameField->GetValue().ToStdString() << std::endl;
    std::string customerName = customerNameField->GetValue().ToStdString();
    
    try {        
        pqxx::work worker( *db_connection );
        pqxx::row row( worker.exec1("select * from create_record_queue(\'" + customerName + "\')") );
        
        wxMessageBox(row["customer_full_name"].as<std::string>() + " " + row["queue_number"].as<std::string>() + " " + row["operator_place"].as<std::string>(),
                 "RESULT", wxOK | wxICON_INFORMATION);
        // std::cout << row["customer_full_name"] << " " << row["queue_number"] << " " << row["operator_place"] << std::endl;        

        worker.commit();
    }
    catch (std::exception &e) {

        wxMessageBox("There are no avaliable operators",
                 "ERROR", wxOK | wxICON_ERROR);

        return;
    }
}

void MyFrame::OnStartOpService(wxCommandEvent& event) {
    wxTextCtrl *textField = static_cast<wxTextCtrl *>(event.GetEventUserData());
    std::string operator_id = textField->GetValue().ToStdString();

    try {
        pqxx::work worker( *db_connection );
        pqxx::row row( worker.exec1("select * from start_operator_service(\'" + operator_id + "\')") );

        wxMessageBox(row["r_customer_full_name"].as<std::string>() + " " 
                   + row["r_queue_number"].as<std::string>() + " " 
                   + row["r_operator_place"].as<std::string>(),
                 "RESULT", wxOK | wxICON_INFORMATION);
        
        worker.commit();
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        wxMessageBox("There are a person processing",
                 "ERROR", wxOK | wxICON_ERROR);

        return;
    }
}
void MyFrame::OnEndOpService(wxCommandEvent& event) {
    wxTextCtrl *textField = static_cast<wxTextCtrl *>(event.GetEventUserData());
    std::string operator_id = textField->GetValue().ToStdString();

    try {
        pqxx::work worker( *db_connection );
        worker.exec0("CALL end_operator_service(\'" + operator_id + "\')");

        wxMessageBox("Operator " + operator_id + " end processing",
                 "RESULT", wxOK | wxICON_INFORMATION);

        worker.commit();
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        wxMessageBox("There are no processing person",
                 "ERROR", wxOK | wxICON_ERROR);

        return;
    }
}
void MyFrame::OnEndOpWorking(wxCommandEvent& event) {
    wxTextCtrl *textField = static_cast<wxTextCtrl *>(event.GetEventUserData());
    std::string operator_id = textField->GetValue().ToStdString();

    try {
        pqxx::work worker( *db_connection );
        worker.exec0("CALL end_operator_working(\'" + operator_id + "\')");

        wxMessageBox("Operator " + operator_id + " end working",
                 "RESULT", wxOK | wxICON_INFORMATION);

        worker.commit();
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        wxMessageBox("There are a person processing",
                 "ERROR", wxOK | wxICON_ERROR);

        return;
    }
}