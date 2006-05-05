//
// Copyright (C) 2006 HLVM Group. All Rights Reserved.
//
// This program is open source software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License (GPL) as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. You should have received a copy of the GPL in a
// file named COPYING that was included with this program; if not, you can
// obtain a copy of the license through the Internet at http://www.fsf.org/
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
////////////////////////////////////////////////////////////////////////////////
/// @file hlvm/AST/Type.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Type
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_AST_CONTAINERTYPE_H
#define HLVM_AST_CONTAINERTYPE_H

#include <hlvm/AST/Type.h>

namespace hlvm {
namespace AST {
  /// This class represents a Type in the HLVM Abstract Syntax Tree.  
  /// A Type defines the format of storage. 
  /// @brief HLVM AST Type Node
  class ContainerType : public Type
  {
    /// @name Constructors
    /// @{
    public:
      ContainerType(
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name, ///< The name of the function
        TypeIDs type_id
      ) : Type(parent,name,type_id) {}
      virtual ~ContainerType();

    /// @}
    /// @name Accessors
    /// @{
    public:

      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const ContainerType*) { return true; }
      static inline bool classof(const Type* T) { return T->isContainerType(); }
    /// @}
    /// @name Data
    /// @{
    protected:
      std::vector<Type*> types_; ///< The contained types
    /// @}
  };

  /// This class represents a storage location that is a pointer to another
  /// type. 
  class PointerType : public ContainerType
  {
    /// @name Constructors
    /// @{
    public:
      PointerType(
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name ///< The name of the function
      ) : ContainerType(parent,name,PointerTypeID) {}
      virtual ~PointerType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const PointerType*) { return true; }
      static inline bool classof(const Type* T) { return T->isPointerType(); }
    /// @}
    /// @name Data
    /// @{
    protected:
    /// @}
  };

  /// This class represents a resizeable, aligned array of some other type. The
  /// Array references a Type that specifies the type of elements in the array.
  class ArrayType : public ContainerType
  {
    /// @name Constructors
    /// @{
    public:
      ArrayType(
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name ///< The name of the function
      ) : ContainerType(parent,name,ArrayTypeID) {}
      virtual ~ArrayType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const ArrayType*) { return true; }
      static inline bool classof(const Type* T) { return T->isArrayType(); }
    /// @}
    /// @name Data
    /// @{
    protected:
    /// @}
  };

  /// This class represents a fixed size, packed vector of some other type.
  /// Where possible, HLVM will attempt to generate code that makes use of a
  /// machines vector instructions to process such types. If not possible, HLVM
  /// will treat the vector the same as an Array.
  class VectorType : public ContainerType
  {
    /// @name Constructors
    /// @{
    public:
      VectorType(
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name ///< The name of the function
      ) : ContainerType(parent,name,VectorTypeID) {}
      virtual ~VectorType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const VectorType*) { return true; }
      static inline bool classof(const Type* T) { return T->isVectorType(); }
    /// @}
    /// @name Data
    /// @{
    protected:
    /// @}
  };

  /// This class represents an HLVM type that is a sequence of data fields 
  /// of varying type. 
  class StructureType : public ContainerType
  {
    /// @name Constructors
    /// @{
    public:
      StructureType(
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name ///< The name of the function
      ) : ContainerType(parent,name,StructureTypeID) {}
      virtual ~StructureType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const StructureType*) { return true; }
      static inline bool classof(const Type* T) { return T->isStructureType(); }
    /// @}
    /// @name Data
    /// @{
    protected:
    /// @}
  };

  /// This class represents an HLVM type that is a sequence of data fields 
  /// of varying type. 
  class SignatureType : public ContainerType
  {
    /// @name Constructors
    /// @{
    public:
      SignatureType(
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name ///< The name of the function
      ) : ContainerType(parent,name,SignatureTypeID) {}
      virtual ~SignatureType();

    /// @}
    /// @name Accessors
    /// @{
    public:
      // Methods to support type inquiry via is, cast, dyn_cast
      static inline bool classof(const SignatureType*) { return true; }
      static inline bool classof(const Type* T) { return T->isSignatureType(); }
    /// @}
    /// @name Data
    /// @{
    protected:
      Type* result_;   ///< The result type of the function signature
      bool isVarArgs;  ///< Indicates variable arguments function
    /// @}
  };
} // AST
} // hlvm
#endif
