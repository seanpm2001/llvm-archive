//===-- hlvm/AST/Program.cpp - AST Program Class ----------------*- C++ -*-===//
//
//                      High Level Virtual Machine (HLVM)
//
// Copyright (C) 2006 Reid Spencer. All Rights Reserved.
//
// This software is free software; you can redistribute it and/or modify it 
// under the terms of the GNU Lesser General Public License as published by 
// the Free Software Foundation; either version 2.1 of the License, or (at 
// your option) any later version.
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for 
// more details.
//
// You should have received a copy of the GNU Lesser General Public License 
// along with this library in the file named LICENSE.txt; if not, write to the 
// Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301 USA
//
//===----------------------------------------------------------------------===//
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::Node.
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Program.h>
#include <hlvm/AST/ContainerType.h>

namespace hlvm {
namespace AST {

SignatureType* Program::initSignature() {
  SignatureType* result = new SignatureType();
  result->setName("_hlvm_ProgramSignature");
  return result;
}
SignatureType* Program::SignatureTy = Program::initSignature();

Program::~Program()
{
}

}}
