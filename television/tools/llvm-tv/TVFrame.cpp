//===-- TVFrame.cpp - Main window class for LLVM-TV -----------------------===//
//
// The gui for llvm-tv.
//
//===----------------------------------------------------------------------===//

#include "TVApplication.h"
#include "TVFrame.h"
#include "TVHtmlWindow.h"
#include "TVTextCtrl.h"
#include "TVTreeItem.h"
#include "llvm/Assembly/Writer.h"
#include <dirent.h>
#include <cassert>
#include <sstream>

/// refreshView - Make sure the display is up-to-date with respect to
/// the list.
///
void TVListCtrl::refreshView () {
  // Clear out the list and then re-add all the items.
  int index = 0;
  ClearAll ();
  for (Items::iterator i = itemList.begin(), e = itemList.end(); i != e; ++i) {
    InsertItem (index, i->label ());
    ++index;
  }
}

/// TreeCtrl constructor that creates the root and adds it to the tree
///
TVTreeCtrl::TVTreeCtrl(wxWindow *parent, const wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
                       long style)
  : wxTreeCtrl(parent, id, pos, size, style) {
  wxTreeItemId rootId = AddRoot(wxT("Snapshots"),
				-1, -1, new TVTreeItemData(wxT("Snapshot Root")));
  
  SetItemImage(rootId, TreeCtrlIcon_FolderOpened, wxTreeItemIcon_Expanded);
}

/// AddSnapshotsToTree - Given a list of snapshots the tree is populated
///
void TVTreeCtrl::AddSnapshotsToTree(std::vector<TVSnapshot> &list) {
  
  wxTreeItemId rootId = GetRootItem();
  for (std::vector<TVSnapshot>::iterator I = list.begin(), E = list.end();
       I != E; ++I) {
    
    // Get the Module associated with this snapshot
    Module *M = I->getModule();
    
    wxTreeItemId id = AppendItem(rootId, I->label(), -1, -1,
                                 new TVTreeModuleItem(I->label(), M));

    // Loop over functions in the module and add them to the tree
    for (Module::iterator I = M->begin(), E = M->end(); I != E; ++I) {
      Function *F = I;
      if (!F->isExternal()) {
        const char *FuncName = F->getName().c_str();
        AppendItem(id, FuncName, -1, -1, new TVTreeFunctionItem(FuncName, I));
      }
    }
    
  }   
}

/// updateSnapshotList - Update the tree with the current snapshot list
///
void TVTreeCtrl::updateSnapshotList(std::vector<TVSnapshot>& list) {
  DeleteChildren(GetRootItem());
  AddSnapshotsToTree(list);
}

/// updateTextDisplayed  - Updates text with the data from the item selected
///
void TVTreeCtrl::updateTextDisplayed() {
  // Get parent and then the text window, then get the selected LLVM object.
  wxWindow *displayWidget = ((wxSplitterWindow *) GetParent())->GetWindow2();
#if defined(NOHTML)
  TVTextCtrl *textDisplay = dynamic_cast<TVTextCtrl *> (displayWidget);
  assert (textDisplay && "could not retrieve display widget");
#else
  TVHtmlWindow *htmlDisplay = dynamic_cast<TVHtmlWindow *> (displayWidget);
  assert (htmlDisplay && "could not retrieve display widget");
#endif

  TVTreeItemData *item = (TVTreeItemData*)GetItemData(GetSelection());

  // Display the assembly language for the selected LLVM object in the
  // right-hand pane.
#if defined(NOHTML)
  textDisplay->displayItem (item);
#else
  htmlDisplay->displayItem (item);
#endif
}

void TVTextCtrl::displayItem (TVTreeItemData *item) {
  std::ostringstream Out;
  item->print (Out);
  Clear ();
  AppendText (Out.str ().c_str ());
}

void TVHtmlWindow::displayItem (TVTreeItemData *item) {
  std::ostringstream Out;
  item->printHTML (Out);
  Hide ();
  SetPage (wxString (""));
  AppendToPage (wxString (Out.str ().c_str ()));
  Show ();
}

/// OnSelChanged - Trigger the text display to be updated with the new
/// item selected
void TVTreeCtrl::OnSelChanged(wxTreeEvent &event) {
  updateTextDisplayed();
}

///==---------------------------------------------------------------------==///

static const wxString Explanation
  ("Click on a Module or Function in the left-hand pane\n"
   "to display its code in the right-hand pane. Then, you\n"
   "can choose from the View menu to see graphical code views.\n"); 

/// createDisplayWidget - Factory Method which returns a new display widget
/// of the appropriate class, with the given parent & initial contents
///
wxWindow *TVFrame::createDisplayWidget (wxWindow *parent, const wxString &init){
  unsigned nohtml;
#if defined(NOHTML)
  nohtml = 1;
#else
  nohtml = 0;
#endif
  if (nohtml) {
    // We'll use a static text control to display LLVM assembly 
    return new TVTextCtrl(parent, LLVM_TV_TEXT_CTRL, init);
  } else {
    // We'll use a HTML viewer to display syntax-highlighted LLVM assembly 
    return new TVHtmlWindow(parent, LLVM_TV_HTML_WINDOW, init);
  }
}

void TVFrame::refreshSnapshotList () {
  // re-load the list of snapshots
  const char *directoryName = mySnapshotDirName.c_str ();
  mySnapshotList.clear ();
  DIR *d = opendir (directoryName);
  if (!d)
    FatalErrorBox ("trying to open directory " + mySnapshotDirName + ": "
                   + strerror(errno));
  while (struct dirent *de = readdir (d))
    if (memcmp(de->d_name, ".", 2) && memcmp(de->d_name, "..", 3))
      mySnapshotList.push_back (mySnapshotDirName + "/" + de->d_name);
  sort (mySnapshotList.begin (), mySnapshotList.end ());

  closedir (d);

  if (myTreeCtrl != 0)
    myTreeCtrl->updateSnapshotList(mySnapshotList);
}

void TVFrame::initializeSnapshotListAndView (std::string dirName) {
  // Initialize the snapshot list
  mySnapshotDirName = dirName;
  refreshSnapshotList ();
  SetStatusText ("Snapshot list has been loaded.");
}

/// TVFrame constructor - used to set up typical appearance of visualizer's
/// top-level window.
///
TVFrame::TVFrame (TVApplication *app, const char *title)
  : wxFrame (NULL, -1, title), myApp (app) {
  // Set up appearance
  CreateStatusBar ();
  SetSize (wxRect (100, 100, 500, 200));
  Show (FALSE);
  splitterWindow = new wxSplitterWindow(this, LLVM_TV_SPLITTER_WINDOW,
                                        wxDefaultPosition, wxDefaultSize,
                                        wxSP_3D);

  // Create tree view of snapshots
  myTreeCtrl = new TVTreeCtrl(splitterWindow, LLVM_TV_TREE_CTRL);
  Resize();

  // Create right-hand pane's display widget
  displayWidget = createDisplayWidget (splitterWindow, Explanation);

  // Split window vertically
  splitterWindow->SplitVertically(myTreeCtrl, displayWidget, 100);
  Show (TRUE);
}

/// OnHelp - display the help dialog
///
void TVFrame::OnHelp (wxCommandEvent &event) {
  wxMessageBox (Explanation, "Help with LLVM-TV");
}

/// OnExit - respond to a request to exit the program.
///
void TVFrame::OnExit (wxCommandEvent &event) {
  myApp->Quit ();
}

/// OnExit - respond to a request to display the About box.
///
void TVFrame::OnAbout (wxCommandEvent &event) {
  wxMessageBox("LLVM Visualization Tool\n\n"
               "By Misha Brukman, Tanya Brethour, and Brian Gaeke\n"
               "Copyright (C) 2004 University of Illinois at Urbana-Champaign\n"
               "http://llvm.cs.uiuc.edu", "About LLVM-TV");
}

/// OnRefresh - respond to a request to refresh the list
///
void TVFrame::OnRefresh (wxCommandEvent &event) {
  // FIXME: Having the list model and the window squashed together into
  // TVFrame sucks. The way it should probably really work is:
  // TVSnapshotList tlm;  // the list model
  // TVListCtrl tlv;      // the list view
  // signal handler catches signal
  // --> calls tlm->changed() 
  //   --> <re-reads directory, refreshes list of snapshots,
  //   -->  kind of like refreshSnapshotList()>
  //   --> calls tlv->redraw()
  //       --> <clears out list of items, re-adds items, OR
  //       -->  adds only changed items, or whatever makes sense,
  //       -->  kind of like TVFrame::refreshView()>
  refreshSnapshotList ();
}

void TVFrame::Resize() {
  wxSize size = GetClientSize();
  myTreeCtrl->SetSize(0, 0, size.x, 2*size.y/3);
}

void TVFrame::CallGraphView(wxCommandEvent &event) {
  // Get the selected LLVM object.
  TVTreeItemData *item =
    (TVTreeItemData *) myTreeCtrl->GetItemData (myTreeCtrl->GetSelection ());

  // Open up a new call graph view window.
  Module *M = item->getModule ();
  if (!M)
    wxMessageBox ("The selected item doesn't have a call graph to view.",
                  "Error", wxOK | wxICON_ERROR, this);
  else
    myApp->OpenCallGraphView (M);
}

void TVFrame::CFGView(wxCommandEvent &event) {
  // Get the selected LLVM object.
  TVTreeItemData *item =
    (TVTreeItemData *) myTreeCtrl->GetItemData (myTreeCtrl->GetSelection ());

  // Open up a new CFG view window.
  Function *F = item->getFunction ();
  if (!F)
    wxMessageBox("The selected item doesn't have a CFG to view.", "Error",
                 wxOK | wxICON_ERROR, this);
  else if (F->isExternal())
    wxMessageBox("External functions have no CFG to view.", "Error",
                 wxOK | wxICON_ERROR, this);
  else
    myApp->OpenCFGView (F);
}

void TVFrame::CodeView(wxCommandEvent &event) {
  // Get the selected LLVM object.
  TVTreeItemData *item =
    (TVTreeItemData *) myTreeCtrl->GetItemData (myTreeCtrl->GetSelection ());

  // Open up a new CFG view window.
  Function *F = item->getFunction ();
  if (!F)
    wxMessageBox("Code can only be viewed on a per-function basis.", "Error",
                  wxOK | wxICON_ERROR, this);
  else if (F->isExternal())
    wxMessageBox("External functions have no code to view.", "Error",
                 wxOK | wxICON_ERROR, this);
  else
    myApp->OpenCodeView(F);
}

BEGIN_EVENT_TABLE (TVFrame, wxFrame)
  EVT_MENU (wxID_EXIT, TVFrame::OnExit)
  EVT_MENU (wxID_ABOUT, TVFrame::OnAbout)
  EVT_MENU (wxID_HELP_CONTENTS, TVFrame::OnHelp)
  EVT_MENU (LLVM_TV_REFRESH, TVFrame::OnRefresh)
  EVT_MENU (LLVM_TV_CALLGRAPHVIEW, TVFrame::CallGraphView)
  EVT_MENU (LLVM_TV_CFGVIEW, TVFrame::CFGView)
  EVT_MENU (LLVM_TV_CODEVIEW, TVFrame::CodeView)
END_EVENT_TABLE ()

BEGIN_EVENT_TABLE(TVTreeCtrl, wxTreeCtrl)
  EVT_TREE_SEL_CHANGED(LLVM_TV_TREE_CTRL, TVTreeCtrl::OnSelChanged)
END_EVENT_TABLE ()
