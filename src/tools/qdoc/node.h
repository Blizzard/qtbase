/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef NODE_H
#define NODE_H

#include <qdir.h>
#include <qmap.h>
#include <qpair.h>
#include <qstringlist.h>

#include "codechunk.h"
#include "doc.h"

QT_BEGIN_NAMESPACE

class Node;
class Tree;
class EnumNode;
class ClassNode;
class InnerNode;
class ExampleNode;
class TypedefNode;
class QmlClassNode;
class QDocDatabase;
class FunctionNode;
class PropertyNode;
class QmlModuleNode;
class CollectionNode;
class QmlPropertyNode;

typedef QList<Node*> NodeList;
typedef QMap<QString, Node*> NodeMap;
typedef QMultiMap<QString, Node*> NodeMultiMap;
typedef QPair<int, int> NodeTypePair;
typedef QList<NodeTypePair> NodeTypeList;
typedef QMap<QString, CollectionNode*> CNMap;
typedef QMultiMap<QString, CollectionNode*> CNMultiMap;

class Node
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::Node)

public:
    enum Type {
        NoType,
        Namespace,
        Class,
        Document,
        Enum,
        Typedef,
        Function,
        Property,
        Variable,
        Group,
        Module,
        QmlType,
        QmlModule,
        QmlPropertyGroup,
        QmlProperty,
        QmlSignal,
        QmlSignalHandler,
        QmlMethod,
        QmlBasicType,
        LastType
    };

    enum SubType {
        NoSubType,
        Example,
        HeaderFile,
        File,
        Image,
        Page,
        ExternalPage,
        DitaMap,
        LastSubtype
    };

    enum Genus { DontCare, CPP, QML };

    enum Access { Public, Protected, Private };

    enum Status {
        Compat,
        Obsolete,
        Deprecated,
        Preliminary,
        Commendable,
        Main,
        Internal
    }; // don't reorder this enum

    enum ThreadSafeness {
        UnspecifiedSafeness,
        NonReentrant,
        Reentrant,
        ThreadSafe
    };

    enum LinkType {
        StartLink,
        NextLink,
        PreviousLink,
        ContentsLink,
        IndexLink /*,
        GlossaryLink,
        CopyrightLink,
        ChapterLink,
        SectionLink,
        SubsectionLink,
        AppendixLink */
    };

    enum PageType {
        NoPageType,
        ApiPage,
        ArticlePage,
        ExamplePage,
        HowToPage,
        OverviewPage,
        TutorialPage,
        FAQPage,
        DitaMapPage,
        OnBeyondZebra
    };

    enum FlagValue {
        FlagValueDefault = -1,
        FlagValueFalse = 0,
        FlagValueTrue = 1
    };

    virtual ~Node();

    QString plainName() const;
    QString plainFullName(const Node* relative = 0) const;
    QString fullName(const Node* relative=0) const;

    const QString& fileNameBase() const { return fileNameBase_; }
    bool hasFileNameBase() const { return !fileNameBase_.isEmpty(); }
    void setFileNameBase(const QString& t) { fileNameBase_ = t; }

    void setAccess(Access access) { access_ = access; }
    void setLocation(const Location& location) { loc_ = location; }
    void setDoc(const Doc& doc, bool replace = false);
    void setStatus(Status status) {
        if (status_ == Obsolete && status == Deprecated)
            return;
        status_ = status;
    }
    void setThreadSafeness(ThreadSafeness safeness) { safeness_ = safeness; }
    void setSince(const QString &since);
    void setRelates(InnerNode* pseudoParent);
    void setModuleName(const QString &name) { moduleName_ = name; }
    void setUrl(const QString& url) { url_ = url; }
    void setTemplateStuff(const QString &templateStuff) { templateStuff_ = templateStuff; }
    void setReconstitutedBrief(const QString &t) { reconstitutedBrief_ = t; }
    void setPageType(PageType t) { pageType_ = t; }
    void setPageType(const QString& t);
    void setParent(InnerNode* n) { parent_ = n; }
    void setIndexNodeFlag() { indexNodeFlag_ = true; }
    virtual void setOutputFileName(const QString& ) { }

    virtual bool isInnerNode() const = 0;
    virtual bool isDocNode() const { return false; }
    virtual bool isCollectionNode() const { return false; }
    virtual bool isGroup() const { return false; }
    virtual bool isModule() const { return false; }
    virtual bool isQmlModule() const { return false; }
    virtual bool isQmlType() const { return false; }
    virtual bool isQmlBasicType() const { return false; }
    virtual bool isExample() const { return false; }
    virtual bool isExampleFile() const { return false; }
    virtual bool isHeaderFile() const { return false; }
    virtual bool isLeaf() const { return false; }
    virtual bool isReimp() const { return false; }
    virtual bool isFunction() const { return false; }
    virtual bool isNamespace() const { return false; }
    virtual bool isClass() const { return false; }
    virtual bool isQmlNode() const { return false; }
    virtual bool isCppNode() const { return false; }
    virtual bool isQtQuickNode() const { return false; }
    virtual bool isAbstract() const { return false; }
    virtual bool isQmlPropertyGroup() const { return false; }
    virtual bool isAttached() const { return false; }
    virtual bool isAlias() const { return false; }
    virtual bool isWrapper() const;
    virtual bool isReadOnly() const { return false; }
    virtual bool isDefault() const { return false; }
    virtual bool isExternalPage() const { return false; }
    virtual void addMember(Node* ) { }
    virtual bool hasMembers() const { return false; }
    virtual bool hasNamespaces() const { return false; }
    virtual bool hasClasses() const { return false; }
    virtual void setAbstract(bool ) { }
    virtual void setWrapper() { }
    virtual Node::Genus genus() const { return DontCare; }
    virtual QString title() const { return name(); }
    virtual QString fullTitle() const { return name(); }
    virtual QString subTitle() const { return QString(); }
    virtual void setTitle(const QString& ) { }
    virtual void setSubTitle(const QString& ) { }
    virtual QmlPropertyNode* hasQmlProperty(const QString& ) const { return 0; }
    virtual QmlPropertyNode* hasQmlProperty(const QString&, bool ) const { return 0; }
    virtual void getMemberNamespaces(NodeMap& ) { }
    virtual void getMemberClasses(NodeMap& ) { }
    virtual bool isInternal() const;
    virtual void setDataType(const QString& ) { }
    virtual void setReadOnly(bool ) { }
    virtual Node* disambiguate(Type , SubType ) { return this; }
    virtual bool wasSeen() const { return false; }
    virtual void appendGroupName(const QString& ) { }
    virtual QString element() const { return QString(); }
    virtual Tree* tree() const;
    virtual void findChildren(const QString& , NodeList& nodes) const { nodes.clear(); }
    bool isIndexNode() const { return indexNodeFlag_; }
    Type type() const { return nodeType_; }
    virtual SubType subType() const { return NoSubType; }
    bool match(const NodeTypeList& types) const;
    InnerNode* parent() const { return parent_; }
    const Node* root() const;
    InnerNode* relates() const { return relatesTo_; }
    const QString& name() const { return name_; }
    QString moduleName() const;
    QString url() const { return url_; }
    virtual QString nameForLists() const { return name_; }
    virtual QString outputFileName() const { return QString(); }
    virtual QString obsoleteLink() const { return QString(); }
    virtual void setObsoleteLink(const QString& ) { };
    virtual void setQtVariable(const QString& ) { }
    virtual QString qtVariable() const { return QString(); }

    const QMap<LinkType, QPair<QString,QString> >& links() const { return linkMap_; }
    void setLink(LinkType linkType, const QString &link, const QString &desc);

    Access access() const { return access_; }
    bool isPrivate() const { return access_ == Private; }
    QString accessString() const;
    const Location& location() const { return loc_; }
    const Doc& doc() const { return doc_; }
    bool hasDoc() const { return !doc_.isEmpty(); }
    Status status() const { return status_; }
    Status inheritedStatus() const;
    bool isObsolete() const { return (status_ == Obsolete); }
    ThreadSafeness threadSafeness() const;
    ThreadSafeness inheritedThreadSafeness() const;
    QString since() const { return since_; }
    QString templateStuff() const { return templateStuff_; }
    const QString& reconstitutedBrief() const { return reconstitutedBrief_; }
    PageType pageType() const { return pageType_; }
    QString pageTypeString() const;
    QString nodeTypeString() const;
    QString nodeSubtypeString() const;
    virtual void addPageKeywords(const QString& ) { }

    void clearRelated() { relatesTo_ = 0; }

    QString guid() const;
    QString extractClassName(const QString &string) const;
    virtual QString qmlTypeName() const { return name_; }
    virtual QString qmlFullBaseName() const { return QString(); }
    virtual QString qmlModuleName() const { return QString(); }
    virtual QString qmlModuleVersion() const { return QString(); }
    virtual QString qmlModuleIdentifier() const { return QString(); }
    virtual void setQmlModuleInfo(const QString& ) { }
    virtual QmlModuleNode* qmlModule() const { return 0; }
    virtual void setQmlModule(QmlModuleNode* ) { }
    virtual ClassNode* classNode() { return 0; }
    virtual void setClassNode(ClassNode* ) { }
    virtual const Node* applyModuleName(const Node* ) const { return 0; }
    virtual QString idNumber() { return "0"; }
    QmlClassNode* qmlClassNode();
    ClassNode* declarativeCppNode();
    const QString& outputSubdirectory() const { return outSubDir_; }
    void setOutputSubdirectory(const QString& t) { outSubDir_ = t; }
    QString fullDocumentName() const;
    static QString cleanId(QString str);
    QString idForNode() const;

    static FlagValue toFlagValue(bool b);
    static bool fromFlagValue(FlagValue fv, bool defaultValue);

    static QString pageTypeString(unsigned t);
    static QString nodeTypeString(unsigned t);
    static QString nodeSubtypeString(unsigned t);
    static int incPropertyGroupCount();
    static void clearPropertyGroupCount();
    static void initialize();
    static Type goal(const QString& t) { return goals_.value(t); }

protected:
    Node(Type type, InnerNode* parent, const QString& name);

private:

    Type nodeType_;
    Access access_;
    ThreadSafeness safeness_;
    PageType pageType_;
    Status status_;
    bool indexNodeFlag_;

    InnerNode* parent_;
    InnerNode* relatesTo_;
    QString name_;
    Location loc_;
    Doc doc_;
    QMap<LinkType, QPair<QString, QString> > linkMap_;
    QString fileNameBase_;
    QString moduleName_;
    QString url_;
    QString since_;
    QString templateStuff_;
    QString reconstitutedBrief_;
    mutable QString uuid_;
    QString outSubDir_;
    static QStringMap operators_;
    static int propertyGroupCount_;
    static QMap<QString,Node::Type> goals_;
};

class InnerNode : public Node
{
public:
    virtual ~InnerNode();

    Node* findChildNode(const QString& name, Node::Genus genus) const;
    //Node* findChildNode(const QString& name, bool qml) const;
    Node* findChildNode(const QString& name, Type type);
    //void findNodes(const QString& name, NodeList& n);
    virtual void findChildren(const QString& name, NodeList& nodes) const;
    FunctionNode* findFunctionNode(const QString& name) const;
    FunctionNode* findFunctionNode(const FunctionNode* clone);
    void addInclude(const QString &include);
    void setIncludes(const QStringList &includes);
    void setOverload(const FunctionNode* func, bool overlode);
    void normalizeOverloads();
    void makeUndocumentedChildrenInternal();
    void deleteChildren();
    void removeFromRelated();

    virtual bool isInnerNode() const { return true; }
    virtual bool isLeaf() const { return false; }
    const EnumNode* findEnumNodeForValue(const QString &enumValue) const;
    const NodeList & childNodes() const { return children_; }
    const NodeList & relatedNodes() const { return related_; }

    int count() const { return children_.size(); }
    int overloadNumber(const FunctionNode* func) const;
    NodeList overloads(const QString &funcName) const;
    const QStringList& includes() const { return includes_; }

    QStringList primaryKeys();
    QStringList secondaryKeys();
    const QStringList& pageKeywords() const { return pageKeywds; }
    virtual void addPageKeywords(const QString& t) { pageKeywds << t; }
    virtual void setOutputFileName(const QString& f) { outputFileName_ = f; }
    virtual QString outputFileName() const { return outputFileName_; }
    virtual QmlPropertyNode* hasQmlProperty(const QString& ) const;
    virtual QmlPropertyNode* hasQmlProperty(const QString&, bool attached) const;
    void addChild(Node* child, const QString& title);
    const QStringList& groupNames() const { return groupNames_; }
    virtual void appendGroupName(const QString& t) { groupNames_.append(t); }
    void printChildren(const QString& title);

protected:
    InnerNode(Type type, InnerNode* parent, const QString& name);

private:
    friend class Node;

    static bool isSameSignature(const FunctionNode* f1, const FunctionNode* f2);
    void addChild(Node* child);
    void removeRelated(Node* pseudoChild);
    void removeChild(Node* child);

    QString outputFileName_;
    QStringList pageKeywds;
    QStringList includes_;
    QStringList groupNames_;
    NodeList children_;
    NodeList enumChildren_;
    NodeList related_;
    QMap<QString, Node*> childMap;
    QMap<QString, Node*> primaryFunctionMap;
    QMap<QString, NodeList> secondaryFunctionMap;
};

class LeafNode : public Node
{
public:
    LeafNode();
    virtual ~LeafNode() { }

    virtual bool isInnerNode() const { return false; }
    virtual bool isLeaf() const { return true; }

protected:
    LeafNode(Type type, InnerNode* parent, const QString& name);
    LeafNode(InnerNode* parent, Type type, const QString& name);
};

class NamespaceNode : public InnerNode
{
public:
    NamespaceNode(InnerNode* parent, const QString& name);
    virtual ~NamespaceNode() { }
    virtual bool isNamespace() const { return true; }
    virtual Tree* tree() const { return (parent() ? parent()->tree() : tree_); }
    virtual bool isCppNode() const { return true; }
    virtual Node::Genus genus() const { return Node::CPP; }
    void setTree(Tree* t) { tree_ = t; }

 private:
    Tree*       tree_;
};

struct RelatedClass
{
    RelatedClass() { }
    // constructor for resolved base class
    RelatedClass(Node::Access access, ClassNode* node)
        : access_(access), node_(node) { }
    // constructor for unresolved base class
    RelatedClass(Node::Access access, const QStringList& path, const QString& signature)
        : access_(access), node_(0), path_(path), signature_(signature) { }
    QString accessString() const;
    bool isPrivate() const { return (access_ == Node::Private); }

    Node::Access        access_;
    ClassNode*          node_;
    QStringList         path_;
    QString             signature_;
};

class ClassNode : public InnerNode
{
public:
    ClassNode(InnerNode* parent, const QString& name);
    virtual ~ClassNode() { }
    virtual bool isClass() const { return true; }
    virtual bool isCppNode() const { return true; }
    virtual bool isWrapper() const { return wrapper_; }
    virtual Node::Genus genus() const { return Node::CPP; }
    virtual QString obsoleteLink() const { return obsoleteLink_; }
    virtual void setObsoleteLink(const QString& t) { obsoleteLink_ = t; }
    virtual void setWrapper() { wrapper_ = true; }

    void addResolvedBaseClass(Access access, ClassNode* node);
    void addDerivedClass(Access access, ClassNode* node);
    void addUnresolvedBaseClass(Access access, const QStringList& path, const QString& signature);
    void fixBaseClasses();
    void fixPropertyUsingBaseClasses(PropertyNode* pn);

    QList<RelatedClass>& baseClasses() { return bases_; }
    QList<RelatedClass>& derivedClasses() { return derived_; }
    QList<RelatedClass>& ignoredBaseClasses() { return ignoredBases_; }

    const QList<RelatedClass> &baseClasses() const { return bases_; }
    const QList<RelatedClass> &derivedClasses() const { return derived_; }
    const QList<RelatedClass> &ignoredBaseClasses() const { return ignoredBases_; }

    QString serviceName() const { return sname; }
    void setServiceName(const QString& value) { sname = value; }
    QmlClassNode* qmlElement() { return qmlelement; }
    void setQmlElement(QmlClassNode* qcn) { qmlelement = qcn; }
    virtual bool isAbstract() const { return abstract_; }
    virtual void setAbstract(bool b) { abstract_ = b; }
    PropertyNode* findPropertyNode(const QString& name);
    QmlClassNode* findQmlBaseNode();

private:
    QList<RelatedClass> bases_;
    QList<RelatedClass> derived_;
    QList<RelatedClass> ignoredBases_;
    bool abstract_;
    bool wrapper_;
    QString sname;
    QString obsoleteLink_;
    QmlClassNode* qmlelement;
};

class DocNode : public InnerNode
{
public:

    DocNode(InnerNode* parent,
             const QString& name,
             SubType subType,
             PageType ptype);
    virtual ~DocNode() { }

    virtual void setTitle(const QString &title);
    virtual void setSubTitle(const QString &subTitle) { subtitle_ = subTitle; }

    SubType subType() const { return nodeSubtype_; }
    virtual QString title() const { return title_; }
    virtual QString fullTitle() const;
    virtual QString subTitle() const;
    virtual QString imageFileName() const { return QString(); }
    virtual QString nameForLists() const { return title(); }
    virtual void setImageFileName(const QString& ) { }
    virtual bool isHeaderFile() const { return (subType() == Node::HeaderFile); }
    virtual bool isExample() const { return (subType() == Node::Example); }
    virtual bool isExampleFile() const { return (parent() && parent()->isExample()); }
    virtual bool isExternalPage() const { return nodeSubtype_ == ExternalPage; }
    virtual bool isDocNode() const { return true; }

protected:
    SubType nodeSubtype_;
    QString title_;
    QString subtitle_;
};

class ExampleNode : public DocNode
{
public:
    ExampleNode(InnerNode* parent, const QString& name)
        : DocNode(parent, name, Node::Example, Node::ExamplePage) { }
    virtual ~ExampleNode() { }
    virtual QString imageFileName() const { return imageFileName_; }
    virtual void setImageFileName(const QString& ifn) { imageFileName_ = ifn; }

private:
    QString imageFileName_;
};

struct ImportRec {
    QString name_;      // module name
    QString version_;   // <major> . <minor>
    QString importId_;  // "as" name
    QString importUri_; // subdirectory of module directory

    ImportRec(const QString& name,
              const QString& version,
              const QString& importId,
              const QString& importUri)
    : name_(name), version_(version), importId_(importId), importUri_(importUri) { }
    QString& name() { return name_; }
    QString& version() { return version_; }
    QString& importId() { return importId_; }
    QString& importUri() { return importUri_; }
    bool isEmpty() const { return name_.isEmpty(); }
};

typedef QList<ImportRec> ImportList;

class QmlClassNode : public InnerNode
{
public:
    QmlClassNode(InnerNode* parent, const QString& name);
    virtual ~QmlClassNode();
    virtual bool isQmlNode() const { return true; }
    virtual bool isQmlType() const { return true; }
    virtual bool isQtQuickNode() const { return (qmlModuleName() == QLatin1String("QtQuick")); }
    virtual ClassNode* classNode() { return cnode_; }
    virtual void setClassNode(ClassNode* cn) { cnode_ = cn; }
    virtual bool isAbstract() const { return abstract_; }
    virtual bool isWrapper() const { return wrapper_; }
    virtual void setAbstract(bool b) { abstract_ = b; }
    virtual void setWrapper() { wrapper_ = true; }
    virtual bool isInternal() const { return (status() == Internal); }
    virtual QString qmlFullBaseName() const;
    virtual QString obsoleteLink() const { return obsoleteLink_; }
    virtual void setObsoleteLink(const QString& t) { obsoleteLink_ = t; };
    virtual QString qmlModuleName() const;
    virtual QString qmlModuleVersion() const;
    virtual QString qmlModuleIdentifier() const;
    virtual QmlModuleNode* qmlModule() const { return qmlModule_; }
    virtual void setQmlModule(QmlModuleNode* t) { qmlModule_ = t; }
    virtual Node::Genus genus() const { return Node::QML; }
    const ImportList& importList() const { return importList_; }
    void setImportList(const ImportList& il) { importList_ = il; }
    const QString& qmlBaseName() const { return qmlBaseName_; }
    void setQmlBaseName(const QString& name) { qmlBaseName_ = name; }
    bool qmlBaseNodeNotSet() const { return (qmlBaseNode_ == 0); }
    QmlClassNode* qmlBaseNode();
    void setQmlBaseNode(QmlClassNode* b) { qmlBaseNode_ = b; }
    void requireCppClass() { cnodeRequired_ = true; }
    bool cppClassRequired() const { return cnodeRequired_; }
    static void addInheritedBy(const QString& base, Node* sub);
    static void subclasses(const QString& base, NodeList& subs);
    static void terminate();

public:
    static bool qmlOnly;
    static QMultiMap<QString,Node*> inheritedBy;

private:
    bool abstract_;
    bool cnodeRequired_;
    bool wrapper_;
    ClassNode*    cnode_;
    QString             qmlBaseName_;
    QString             obsoleteLink_;
    QmlModuleNode*      qmlModule_;
    QmlClassNode*       qmlBaseNode_;
    ImportList          importList_;
};

class QmlBasicTypeNode : public InnerNode
{
public:
    QmlBasicTypeNode(InnerNode* parent,
                     const QString& name);
    virtual ~QmlBasicTypeNode() { }
    virtual bool isQmlNode() const { return true; }
    virtual bool isQmlBasicType() const { return true; }
    virtual Node::Genus genus() const { return Node::QML; }
};

class QmlPropertyGroupNode : public InnerNode
{
public:
    QmlPropertyGroupNode(QmlClassNode* parent, const QString& name);
    virtual ~QmlPropertyGroupNode() { }
    virtual bool isQmlNode() const { return true; }
    virtual bool isQtQuickNode() const { return parent()->isQtQuickNode(); }
    virtual QString qmlTypeName() const { return parent()->qmlTypeName(); }
    virtual QString qmlModuleName() const { return parent()->qmlModuleName(); }
    virtual QString qmlModuleVersion() const { return parent()->qmlModuleVersion(); }
    virtual QString qmlModuleIdentifier() const { return parent()->qmlModuleIdentifier(); }
    virtual QString idNumber();
    virtual bool isQmlPropertyGroup() const { return true; }
    virtual Node::Genus genus() const { return Node::QML; }

    virtual QString element() const { return parent()->name(); }

 private:
    int     idNumber_;
};

class QmlPropertyNode : public LeafNode
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::QmlPropertyNode)

public:
    QmlPropertyNode(InnerNode *parent,
                    const QString& name,
                    const QString& type,
                    bool attached);
    virtual ~QmlPropertyNode() { }

    virtual Node::Genus genus() const { return Node::QML; }
    virtual void setDataType(const QString& dataType) { type_ = dataType; }
    void setStored(bool stored) { stored_ = toFlagValue(stored); }
    void setDesignable(bool designable) { designable_ = toFlagValue(designable); }
    virtual void setReadOnly(bool ro) { readOnly_ = toFlagValue(ro); }
    void setDefault() { isdefault_ = true; }

    const QString &dataType() const { return type_; }
    QString qualifiedDataType() const { return type_; }
    bool isReadOnlySet() const { return (readOnly_ != FlagValueDefault); }
    bool isStored() const { return fromFlagValue(stored_,true); }
    bool isDesignable() const { return fromFlagValue(designable_,false); }
    bool isWritable();
    virtual bool isDefault() const { return isdefault_; }
    virtual bool isReadOnly() const { return fromFlagValue(readOnly_,false); }
    virtual bool isAlias() const { return isAlias_; }
    virtual bool isAttached() const { return attached_; }
    virtual bool isQmlNode() const { return true; }
    virtual bool isQtQuickNode() const { return parent()->isQtQuickNode(); }
    virtual QString qmlTypeName() const { return parent()->qmlTypeName(); }
    virtual QString qmlModuleName() const { return parent()->qmlModuleName(); }
    virtual QString qmlModuleVersion() const { return parent()->qmlModuleVersion(); }
    virtual QString qmlModuleIdentifier() const { return parent()->qmlModuleIdentifier(); }
    virtual QString element() const;

 private:
    PropertyNode* findCorrespondingCppProperty();

private:
    QString type_;
    FlagValue   stored_;
    FlagValue   designable_;
    bool    isAlias_;
    bool    isdefault_;
    bool    attached_;
    FlagValue   readOnly_;
};

class EnumItem
{
public:
    EnumItem() { }
    EnumItem(const QString& name, const QString& value)
        : nam(name), val(value) { }

    const QString& name() const { return nam; }
    const QString& value() const { return val; }

private:
    QString nam;
    QString val;
};

class EnumNode : public LeafNode
{
public:
    EnumNode(InnerNode* parent, const QString& name);
    virtual ~EnumNode() { }

    virtual Node::Genus genus() const { return Node::CPP; }
    virtual bool isCppNode() const { return true; }
    void addItem(const EnumItem& item);
    void setFlagsType(TypedefNode* typedeff);
    bool hasItem(const QString &name) const { return names.contains(name); }

    const QList<EnumItem>& items() const { return itms; }
    Access itemAccess(const QString& name) const;
    const TypedefNode* flagsType() const { return ft; }
    QString itemValue(const QString &name) const;

private:
    QList<EnumItem> itms;
    QSet<QString> names;
    const TypedefNode* ft;
};

class TypedefNode : public LeafNode
{
public:
    TypedefNode(InnerNode* parent, const QString& name);
    virtual ~TypedefNode() { }

    virtual Node::Genus genus() const { return Node::CPP; }
    virtual bool isCppNode() const { return true; }
    const EnumNode* associatedEnum() const { return ae; }

private:
    void setAssociatedEnum(const EnumNode* enume);

    friend class EnumNode;

    const EnumNode* ae;
};

inline void EnumNode::setFlagsType(TypedefNode* typedeff)
{
    ft = typedeff;
    typedeff->setAssociatedEnum(this);
}


class Parameter
{
public:
    Parameter() {}
    Parameter(const QString& leftType,
              const QString& rightType = QString(),
              const QString& name = QString(),
              const QString& defaultValue = QString());
    Parameter(const Parameter& p);

    Parameter& operator=(const Parameter& p);

    void setName(const QString& name) { nam = name; }

    bool hasType() const { return lef.length() + rig.length() > 0; }
    const QString& leftType() const { return lef; }
    const QString& rightType() const { return rig; }
    const QString& name() const { return nam; }
    const QString& defaultValue() const { return def; }

    QString reconstruct(bool value = false) const;

private:
    QString lef;
    QString rig;
    QString nam;
    QString def;
};

class FunctionNode : public LeafNode
{
public:
    enum Metaness {
        Plain,
        Signal,
        Slot,
        Ctor,
        Dtor,
        MacroWithParams,
        MacroWithoutParams,
        Native };
    enum Virtualness { NonVirtual, ImpureVirtual, PureVirtual };

    FunctionNode(InnerNode* parent, const QString &name);
    FunctionNode(Type type, InnerNode* parent, const QString &name, bool attached);
    virtual ~FunctionNode() { }

    void setReturnType(const QString& returnType) { rt = returnType; }
    void setParentPath(const QStringList& parentPath) { pp = parentPath; }
    void setMetaness(Metaness metaness) { met = metaness; }
    void setVirtualness(Virtualness virtualness);
    void setConst(bool conste) { con = conste; }
    void setStatic(bool statique) { sta = statique; }
    void setOverload(bool overlode);
    void setReimp(bool r);
    void addParameter(const Parameter& parameter);
    inline void setParameters(const QList<Parameter>& parameters);
    void borrowParameterNames(const FunctionNode* source);
    void setReimplementedFrom(FunctionNode* from);

    const QString& returnType() const { return rt; }
    Metaness metaness() const { return met; }
    bool isMacro() const {
        return met == MacroWithParams || met == MacroWithoutParams;
    }
    Virtualness virtualness() const { return vir; }
    bool isConst() const { return con; }
    bool isStatic() const { return sta; }
    bool isOverload() const { return ove; }
    bool isReimp() const { return reimp; }
    bool isFunction() const { return true; }
    int overloadNumber() const;
    const QList<Parameter>& parameters() const { return params; }
    QStringList parameterNames() const;
    QString rawParameters(bool names = false, bool values = false) const;
    const FunctionNode* reimplementedFrom() const { return rf; }
    const QList<FunctionNode*> &reimplementedBy() const { return rb; }
    const PropertyNode* associatedProperty() const { return ap; }
    const QStringList& parentPath() const { return pp; }

    QStringList reconstructParams(bool values = false) const;
    QString signature(bool values = false) const;
    virtual QString element() const { return parent()->name(); }
    virtual bool isAttached() const { return attached_; }
    virtual bool isQmlNode() const {
        return ((type() == QmlSignal) ||
                (type() == QmlMethod) ||
                (type() == QmlSignalHandler));
    }
    virtual bool isCppNode() const { return !isQmlNode(); }
    virtual Node::Genus genus() const { return (isQmlNode() ? Node::QML : Node::CPP); }
    virtual bool isQtQuickNode() const { return parent()->isQtQuickNode(); }
    virtual QString qmlTypeName() const { return parent()->qmlTypeName(); }
    virtual QString qmlModuleName() const { return parent()->qmlModuleName(); }
    virtual QString qmlModuleVersion() const { return parent()->qmlModuleVersion(); }
    virtual QString qmlModuleIdentifier() const { return parent()->qmlModuleIdentifier(); }

    void debug() const;

private:
    void setAssociatedProperty(PropertyNode* property);

    friend class InnerNode;
    friend class PropertyNode;

    QString     rt;
    QStringList pp;
    Metaness    met;
    Virtualness vir;
    bool con : 1;
    bool sta : 1;
    bool ove : 1;
    bool reimp: 1;
    bool attached_: 1;
    QList<Parameter> params;
    const FunctionNode* rf;
    const PropertyNode* ap;
    QList<FunctionNode*> rb;
};

class PropertyNode : public LeafNode
{
public:
    enum FunctionRole { Getter, Setter, Resetter, Notifier };
    enum { NumFunctionRoles = Notifier + 1 };

    PropertyNode(InnerNode* parent, const QString& name);
    virtual ~PropertyNode() { }

    virtual Node::Genus genus() const { return Node::CPP; }
    virtual bool isCppNode() const { return true; }
    virtual void setDataType(const QString& dataType) { type_ = dataType; }
    void addFunction(FunctionNode* function, FunctionRole role);
    void addSignal(FunctionNode* function, FunctionRole role);
    void setStored(bool stored) { stored_ = toFlagValue(stored); }
    void setDesignable(bool designable) { designable_ = toFlagValue(designable); }
    void setScriptable(bool scriptable) { scriptable_ = toFlagValue(scriptable); }
    void setWritable(bool writable) { writable_ = toFlagValue(writable); }
    void setUser(bool user) { user_ = toFlagValue(user); }
    void setOverriddenFrom(const PropertyNode* baseProperty);
    void setRuntimeDesFunc(const QString& rdf) { runtimeDesFunc = rdf; }
    void setRuntimeScrFunc(const QString& scrf) { runtimeScrFunc = scrf; }
    void setConstant() { cst = true; }
    void setFinal() { fnl = true; }
    void setRevision(int revision) { rev = revision; }

    const QString &dataType() const { return type_; }
    QString qualifiedDataType() const;
    NodeList functions() const;
    NodeList functions(FunctionRole role) const { return funcs[(int)role]; }
    NodeList getters() const { return functions(Getter); }
    NodeList setters() const { return functions(Setter); }
    NodeList resetters() const { return functions(Resetter); }
    NodeList notifiers() const { return functions(Notifier); }
    bool isStored() const { return fromFlagValue(stored_, storedDefault()); }
    bool isDesignable() const { return fromFlagValue(designable_, designableDefault()); }
    bool isScriptable() const { return fromFlagValue(scriptable_, scriptableDefault()); }
    const QString& runtimeDesignabilityFunction() const { return runtimeDesFunc; }
    const QString& runtimeScriptabilityFunction() const { return runtimeScrFunc; }
    bool isWritable() const { return fromFlagValue(writable_, writableDefault()); }
    bool isUser() const { return fromFlagValue(user_, userDefault()); }
    bool isConstant() const { return cst; }
    bool isFinal() const { return fnl; }
    const PropertyNode* overriddenFrom() const { return overrides; }

    bool storedDefault() const { return true; }
    bool userDefault() const { return false; }
    bool designableDefault() const { return !setters().isEmpty(); }
    bool scriptableDefault() const { return true; }
    bool writableDefault() const { return !setters().isEmpty(); }

private:
    QString type_;
    QString runtimeDesFunc;
    QString runtimeScrFunc;
    NodeList funcs[NumFunctionRoles];
    FlagValue stored_;
    FlagValue designable_;
    FlagValue scriptable_;
    FlagValue writable_;
    FlagValue user_;
    bool cst;
    bool fnl;
    int rev;
    const PropertyNode* overrides;
};

inline void FunctionNode::setParameters(const QList<Parameter> &parameters)
{
    params = parameters;
}

inline void PropertyNode::addFunction(FunctionNode* function, FunctionRole role)
{
    funcs[(int)role].append(function);
    function->setAssociatedProperty(this);
}

inline void PropertyNode::addSignal(FunctionNode* function, FunctionRole role)
{
    funcs[(int)role].append(function);
    function->setAssociatedProperty(this);
}

inline NodeList PropertyNode::functions() const
{
    NodeList list;
    for (int i = 0; i < NumFunctionRoles; ++i)
        list += funcs[i];
    return list;
}

class VariableNode : public LeafNode
{
public:
    VariableNode(InnerNode* parent, const QString &name);
    virtual ~VariableNode() { }

    virtual Node::Genus genus() const { return Node::CPP; }
    virtual bool isCppNode() const { return true; }
    void setLeftType(const QString &leftType) { lt = leftType; }
    void setRightType(const QString &rightType) { rt = rightType; }
    void setStatic(bool statique) { sta = statique; }

    const QString &leftType() const { return lt; }
    const QString &rightType() const { return rt; }
    QString dataType() const { return lt + rt; }
    bool isStatic() const { return sta; }

private:
    QString lt;
    QString rt;
    bool sta;
};

inline VariableNode::VariableNode(InnerNode* parent, const QString &name)
    : LeafNode(Variable, parent, name), sta(false)
{
    // nothing.
}

class DitaMapNode : public DocNode
{
public:
    DitaMapNode(InnerNode* parent, const QString& name)
        : DocNode(parent, name, Node::Page, Node::DitaMapPage) { }
    virtual ~DitaMapNode() { }

    const DitaRefList& map() const { return doc().ditamap(); }
};

class CollectionNode : public InnerNode
{
 public:
    CollectionNode(Type type, InnerNode* parent, const QString& name)
        : InnerNode(type, parent, name), seen_(false) {
        setPageType(Node::OverviewPage);
    }
    virtual ~CollectionNode() { }

    virtual bool isCollectionNode() const { return true; }
    virtual void addMember(Node* node);
    virtual bool hasMembers() const;
    virtual bool hasNamespaces() const;
    virtual bool hasClasses() const;
    virtual void getMemberNamespaces(NodeMap& out);
    virtual void getMemberClasses(NodeMap& out);
    virtual bool wasSeen() const { return seen_; }
    virtual QString title() const { return title_; }
    virtual QString subTitle() const { return subtitle_; }
    virtual QString fullTitle() const { return title_; }
    virtual QString nameForLists() const { return title_; }
    virtual void setTitle(const QString &title);
    virtual void setSubTitle(const QString &subTitle) { subtitle_ = subTitle; }

    const NodeList& members() const { return members_; }
    void printMembers(const QString& title);

    void markSeen() { seen_ = true; }
    void markNotSeen() { seen_ = false; }

 private:
    bool        seen_;
    QString     title_;
    QString     subtitle_;
    NodeList    members_;
};

class GroupNode : public CollectionNode
{
 public:
    GroupNode(InnerNode* parent, const QString& name)
        : CollectionNode(Node::Group, parent, name) { }
    virtual ~GroupNode() { }

    virtual bool isGroup() const { return true; }
};

class ModuleNode : public CollectionNode
{
 public:
    ModuleNode(InnerNode* parent, const QString& name)
        : CollectionNode(Node::Module, parent, name) { }
    virtual ~ModuleNode() { }

    virtual bool isModule() const { return true; }
    virtual bool isCppNode() const { return true; }
    virtual void setQtVariable(const QString& v) { qtVariable_ = v; }
    virtual QString qtVariable() const { return qtVariable_; }

 private:
    QString     qtVariable_;
};

class QmlModuleNode : public CollectionNode
{
 public:
    QmlModuleNode(InnerNode* parent, const QString& name)
        : CollectionNode(Node::QmlModule, parent, name) { }
    virtual ~QmlModuleNode() { }

    virtual bool isQmlNode() const { return true; }
    virtual bool isQmlModule() const { return true; }
    virtual QString qmlModuleName() const { return qmlModuleName_; }
    virtual QString qmlModuleVersion() const {
        return qmlModuleVersionMajor_ + "." + qmlModuleVersionMinor_;
    }
    virtual QString qmlModuleIdentifier() const {
        return qmlModuleName_ + qmlModuleVersionMajor_;
    }
    virtual void setQmlModuleInfo(const QString& );
    virtual void setQtVariable(const QString& v) { qtVariable_ = v; }
    virtual QString qtVariable() const { return qtVariable_; }

 private:
    QString     qmlModuleName_;
    QString     qmlModuleVersionMajor_;
    QString     qmlModuleVersionMinor_;
    QString     qtVariable_;
};

QT_END_NAMESPACE

#endif
