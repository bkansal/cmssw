#ifndef FWCore_Framework_ProductRegistryHelper_h
#define FWCore_Framework_ProductRegistryHelper_h

/*----------------------------------------------------------------------
  
ProductRegistryHelper: 

----------------------------------------------------------------------*/

#include "FWCore/Utilities/interface/TypeID.h"
#include "FWCore/Utilities/interface/Transition.h"
#include "FWCore/Utilities/interface/EDPutToken.h"
#include "DataFormats/Provenance/interface/BranchType.h"
#include <string>
#include <vector>

namespace edm {
  class ModuleDescription;
  class ProductRegistry;
  class ProductRegistryHelper {
  public:

    ProductRegistryHelper() : typeLabelList_() {}
    ~ProductRegistryHelper();
 
    struct TypeLabelItem {
      TypeLabelItem (Transition const& transition, TypeID const& tid, std::string pin) :
	      transition_(transition),
        typeID_(tid),
        productInstanceName_(std::move(pin)),
        branchAlias_() {}
      Transition transition_;
      TypeID typeID_;
      std::string productInstanceName_;
      std::string branchAlias_;
    };

    struct BranchAliasSetter {
      BranchAliasSetter(TypeLabelItem& iItem, EDPutToken iToken):
      value_(iItem), token_(std::move(iToken)) {}
      
      BranchAliasSetter& setBranchAlias(std::string alias) {
        value_.branchAlias_ = std::move(alias);
        return *this;
      }
      TypeLabelItem& value_;
      EDPutToken token_;
      
      operator EDPutToken() { return token_;}
    };

    template <typename T>
    struct BranchAliasSetterT {
      BranchAliasSetterT(TypeLabelItem& iItem, EDPutTokenT<T> iToken):
      value_(iItem), token_(std::move(iToken)) {}

      BranchAliasSetterT( BranchAliasSetter&& iS):
      value_(iS.value_), token_(iS.token_.index()) {}
      
      BranchAliasSetterT<T>& setBranchAlias(std::string alias) {
        value_.branchAlias_ = std::move(alias);
        return *this;
      }
      TypeLabelItem& value_;
      EDPutTokenT<T> token_;
      
      operator EDPutTokenT<T>() { return token_;}
      operator EDPutToken() { return EDPutToken(token_.index()); }
    };

    typedef std::vector<TypeLabelItem> TypeLabelList;

    /// used by the fwk to register the list of products of this module 
    TypeLabelList const& typeLabelList() const;

    static
    void addToRegistry(TypeLabelList::const_iterator const& iBegin,
                             TypeLabelList::const_iterator const& iEnd,
                             ModuleDescription const& iDesc,
                             ProductRegistry& iReg,
                             bool iIsListener=false);

    /// declare what type of product will make and with which optional label 
    /** the statement
        \code
           produces<ProductType>("optlabel");
        \endcode
        should be added to the producer ctor for every product */


    template <class ProductType> 
    BranchAliasSetterT<ProductType> produces() {
      return produces<ProductType, InEvent>(std::string());
    }

    template <class ProductType> 
    BranchAliasSetterT<ProductType> produces(std::string const& instanceName) {
      return produces<ProductType, InEvent>(instanceName);
    }

    template <typename ProductType, BranchType B> 
    BranchAliasSetterT<ProductType> produces() {
      return produces<ProductType, B>(std::string());
    }

    template <typename ProductType, BranchType B> 
    BranchAliasSetterT<ProductType> produces(std::string const& instanceName) {
      TypeID tid(typeid(ProductType));
      return BranchAliasSetterT<ProductType>{produces<B>(tid,instanceName)};
    }

    template <typename ProductType, Transition B>
    BranchAliasSetterT<ProductType> produces() {
      return produces<ProductType, B>(std::string());
    }
    
    template <typename ProductType, Transition B>
    BranchAliasSetterT<ProductType> produces(std::string const& instanceName) {
      TypeID tid(typeid(ProductType));
      return BranchAliasSetterT<ProductType>{produces<B>(tid,instanceName)};
    }

   
    BranchAliasSetter produces(const TypeID& id, std::string const& instanceName=std::string()) {
      return produces<Transition::Event>(id,instanceName);
    }

    template <BranchType B>
    BranchAliasSetter produces(const TypeID& id, std::string const& instanceName=std::string()) {
      unsigned int index =typeLabelList_.size();
       typeLabelList_.emplace_back(convertToTransition(B), id, instanceName);
      return BranchAliasSetter{typeLabelList_.back(),EDPutToken{static_cast<unsigned int>(index)}};
    }
    template <Transition B>
    BranchAliasSetter produces(const TypeID& id, std::string const& instanceName=std::string()) {
      unsigned int index =typeLabelList_.size();
      typeLabelList_.emplace_back(B, id, instanceName);
      return BranchAliasSetter{typeLabelList_.back(),EDPutToken{ index }};
    }

  private:
    TypeLabelList typeLabelList_;
  };


}

#endif
