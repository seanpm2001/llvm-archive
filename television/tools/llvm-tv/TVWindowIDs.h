#ifndef TVWINDOWIDS_H
#define TVWINDOWIDS_H

#include "wx/wx.h"

/// Event IDs we use in the application
///
enum { 
  LLVM_TV_REFRESH = wxID_HIGHEST + 1,
  LLVM_TV_TREE_CTRL,
  LLVM_TV_TEXT_CTRL,
  LLVM_TV_HTML_WINDOW,
  LLVM_TV_SPLITTER_WINDOW,
  LLVM_TV_CALLGRAPHVIEW,
  LLVM_TV_CFGVIEW,
  LLVM_TV_BUDS_VIEW,
  LLVM_TV_TDDS_VIEW,
  LLVM_TV_LOCALDS_VIEW,
  LLVM_TV_CODEVIEW,
  LLVM_TV_CODEVIEW_LIST,
  LLVM_TV_NOTEBOOK
};

#endif // TVWINDOWIDS_H
