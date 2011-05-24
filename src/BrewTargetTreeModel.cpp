/*
 * BrewTargetTreeModel.cpp is part of Brewtarget and was written by Mik
 * Firestone (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QAbstractItemModel>
#include <Qt>
#include <QObject>

#include "brewtarget.h"
#include "recipe.h"
#include "BrewTargetTreeItem.h"
#include "BrewTargetTreeModel.h"
#include "BrewTargetTreeView.h"

BrewTargetTreeModel::BrewTargetTreeModel(BrewTargetTreeView *parent)
   : QAbstractItemModel(parent)
{
   // Initialize the tree structure
    rootItem = new BrewTargetTreeItem();
    rootItem->insertChildren(0,1,BrewTargetTreeItem::RECIPE);
    rootItem->insertChildren(1,1,BrewTargetTreeItem::EQUIPMENT);
    rootItem->insertChildren(2,1,BrewTargetTreeItem::FERMENTABLE);
    rootItem->insertChildren(3,1,BrewTargetTreeItem::HOP);
    rootItem->insertChildren(4,1,BrewTargetTreeItem::MISC);
    rootItem->insertChildren(5,1,BrewTargetTreeItem::YEAST);

   parentTree = parent;
}

BrewTargetTreeModel::~BrewTargetTreeModel()
{
   delete rootItem;
}

BrewTargetTreeItem *BrewTargetTreeModel::getItem( const QModelIndex &index ) const
{
   if ( index.isValid())
   {
      BrewTargetTreeItem *item = static_cast<BrewTargetTreeItem*>(index.internalPointer());
      if (item)
         return item;
   }

   return rootItem;
}

int BrewTargetTreeModel::rowCount(const QModelIndex &parent) const
{
    if (! parent.isValid())
        return rootItem->childCount();

   BrewTargetTreeItem *pItem = getItem(parent);

   return pItem->childCount();
}

int BrewTargetTreeModel::columnCount( const QModelIndex &parent) const
{
    return BrewTargetTreeItem::RECIPENUMCOLS;
}

Qt::ItemFlags BrewTargetTreeModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
      return 0;

   return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex BrewTargetTreeModel::index( int row, int column, const QModelIndex &parent) const
{
   BrewTargetTreeItem *pItem, *cItem;

   if ( parent.isValid() && parent.column() != 0)
      return QModelIndex();

   pItem = getItem(parent);
   cItem = pItem->child(row);

   if (cItem)
      return createIndex(row,column,cItem);
   else
      return QModelIndex();
}

QModelIndex BrewTargetTreeModel::parent(const QModelIndex &index) const
{
   BrewTargetTreeItem *pItem, *cItem;

   if (!index.isValid())
      return QModelIndex();

   cItem = getItem(index);
   pItem = cItem->parent();

   if (pItem == rootItem)
      return QModelIndex();

   return createIndex(pItem->childNumber(),0,pItem);
}

QVariant BrewTargetTreeModel::data(const QModelIndex &index, int role) const
{
   if ( !rootItem || !index.isValid() || index.column() < 0 || index.column() >= BrewTargetTreeItem::RECIPENUMCOLS)
      return QVariant();

   if ( role != Qt::DisplayRole && role != Qt::EditRole)
      return QVariant();

   BrewTargetTreeItem *item = getItem(index);
   return item->getData(index.column());
}

// This is still wrong.  Working on it.  I think if I stablize everything on
// the idea of "name, form and use" I can get this to work
QVariant BrewTargetTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
      return QVariant();

   switch(section)
   {
   case BrewTargetTreeItem::RECIPENAMECOL:
      return QVariant(tr("Name"));
   case BrewTargetTreeItem::RECIPEBREWDATECOL:
      return QVariant(tr("Brew Date"));
   case BrewTargetTreeItem::RECIPESTYLECOL:
      return QVariant(tr("Style"));
   default:
      Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad column: %1").arg(section));
      return QVariant();
   }
}

bool BrewTargetTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
   if ( ! parent.isValid() )
      return false;

   BrewTargetTreeItem *pItem = getItem(parent);
   int type = pItem->getType();

   BrewTargetTreeItem *gpItem = pItem->parent();

   if (gpItem->getType() == BrewTargetTreeItem::RECIPE && pItem->getType() == BrewTargetTreeItem::RECIPE)
      type = BrewTargetTreeItem::BREWNOTE;

   bool success = true;

   beginInsertRows(parent, row, row + count - 1);
   success = pItem->insertChildren(row, count, type);
   endInsertRows();

   return success;
}

bool BrewTargetTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
   BrewTargetTreeItem *pItem = getItem(parent);
   bool success = true;
    
   beginRemoveRows(parent, row, row + count -1 );
   success = pItem->removeChildren(row,count);
   endRemoveRows();

   return success;
}

QModelIndex BrewTargetTreeModel::findRecipe(Recipe* rec)
{
   BrewTargetTreeItem* pItem = rootItem->child(BrewTargetTreeItem::RECIPE);
   int i;

   if (! rec )
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getRecipe() == rec )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findEquipment(Equipment* kit)
{
   BrewTargetTreeItem* pItem = rootItem->child(BrewTargetTreeItem::EQUIPMENT);
   int i;

   if (! kit )
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getEquipment() == kit )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findFermentable(Fermentable* ferm)
{
   BrewTargetTreeItem* pItem = rootItem->child(BrewTargetTreeItem::FERMENTABLE);
   int i;

   if (! ferm )
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getFermentable() == ferm )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findHop(Hop* hop)
{
   BrewTargetTreeItem* pItem = rootItem->child(BrewTargetTreeItem::HOP);
   int i;

   if (! hop )
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getHop() == hop )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findMisc(Misc* misc)
{
   BrewTargetTreeItem* pItem = rootItem->child(BrewTargetTreeItem::MISC);
   int i;

   if (! misc )
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getMisc() == misc )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findYeast(Yeast* yeast)
{
   BrewTargetTreeItem* pItem = rootItem->child(BrewTargetTreeItem::YEAST);
   int i;

   if (! yeast )
      return createIndex(0,0,pItem);

   for(i=0; i < pItem->childCount(); ++i)
   {
      if ( pItem->child(i)->getYeast() == yeast )
         return createIndex(i,0,pItem->child(i));
   }
   return QModelIndex();
}

QModelIndex BrewTargetTreeModel::findBrewNote(BrewNote* bNote)
{
   Recipe *parent = bNote->getParent();
   int i,j; 

   if (! bNote )
      return QModelIndex();

   j = -1;
   for (i=0; static_cast<unsigned int>(i) < parent->getNumBrewNotes();++i)
      if ( parent->getBrewNote(i) == bNote )
         j = i;

   if ( j != -1 )
      return createIndex(j,0, getItem(findRecipe(parent)));

   return QModelIndex();
}

void BrewTargetTreeModel::startObservingDB()
{
   dbObs = Database::getDatabase();
   setObserved(dbObs);

   loadTreeModel(DBALL);
}

void BrewTargetTreeModel::loadTreeModel(int reload)
{
   int i;
   int rows;

   if( ! Database::isInitialized() )
      return;

   if ( reload == DBALL || reload == DBRECIPE)
   {
      BrewTargetTreeItem* local = rootItem->child(BrewTargetTreeItem::RECIPE);

      QList<Recipe*>::iterator it;

      rows = dbObs->getNumRecipes();

      // Insert all the rows
      insertRows(0,rows, createIndex(BrewTargetTreeItem::RECIPE,0,local));

       // And set the data
      for( i = 0, it = dbObs->getRecipeBegin(); it != dbObs->getRecipeEnd(); ++it, ++i )
      {
         BrewTargetTreeItem* temp = local->child(i);
         Recipe* foo = *it;

         // Watch the recipe for updates.
         addObserved(foo);

         temp->setData(BrewTargetTreeItem::RECIPE,foo);

         // If we have brewnotes, set them up here.
         // Ouch.  My head hurts.
         if ( foo->getNumBrewNotes() > 0 ) {
            insertRows(0,foo->getNumBrewNotes(), createIndex(i,0,temp));
            for (unsigned int j=0; j < foo->getNumBrewNotes(); ++j)
            {
               BrewTargetTreeItem* bar = temp->child(j);
               bar->setData(BrewTargetTreeItem::BREWNOTE,foo->getBrewNote(j));
            }
         }
      }
   }

   if ( reload == DBALL || reload == DBEQUIP)
   {
      BrewTargetTreeItem* local = rootItem->child(BrewTargetTreeItem::EQUIPMENT);
      QList<Equipment*>::iterator it;

      rows = dbObs->getNumEquipments();
      insertRows(0,rows, createIndex(BrewTargetTreeItem::EQUIPMENT,0,local));

      for( i = 0, it = dbObs->getEquipmentBegin(); it != dbObs->getEquipmentEnd(); ++it, ++i )
      {
         BrewTargetTreeItem* temp = local->child(i);
         temp->setData(BrewTargetTreeItem::EQUIPMENT,*it);
      }
   }

   if ( reload == DBALL || reload == DBFERM)
   {
      BrewTargetTreeItem* local = rootItem->child(BrewTargetTreeItem::FERMENTABLE);
      QList<Fermentable*>::iterator it;

      rows = dbObs->getNumFermentables();
      insertRows(0,rows,createIndex(BrewTargetTreeItem::FERMENTABLE,0,local));

      for( i = 0, it = dbObs->getFermentableBegin(); it != dbObs->getFermentableEnd(); ++it, ++i )
      {
         BrewTargetTreeItem* temp = local->child(i);
         temp->setData(BrewTargetTreeItem::FERMENTABLE,*it);
      }
   }

   if ( reload == DBALL || reload == DBHOP)
   {
      BrewTargetTreeItem* local = rootItem->child(BrewTargetTreeItem::HOP);
      QList<Hop*>::iterator it;

      rows = dbObs->getNumHops();
      insertRows(0,rows,createIndex(BrewTargetTreeItem::HOP,0,local));

      for( i = 0, it = dbObs->getHopBegin(); it != dbObs->getHopEnd(); ++it, ++i )
      {
         BrewTargetTreeItem* temp = local->child(i);
         temp->setData(BrewTargetTreeItem::HOP,*it);
      }
   }

   if ( reload == DBALL || reload == DBMISC)
   {
      BrewTargetTreeItem* local = rootItem->child(BrewTargetTreeItem::MISC);
      QList<Misc*>::iterator it;

      rows = dbObs->getNumMiscs();
      insertRows(0,rows,createIndex(BrewTargetTreeItem::MISC,0,local));

      for( i = 0, it = dbObs->getMiscBegin(); it != dbObs->getMiscEnd(); ++it, ++i )
      {
         BrewTargetTreeItem* temp = local->child(i);
         temp->setData(BrewTargetTreeItem::MISC,*it);
      }
   }

   if ( reload == DBALL || reload == DBYEAST)
   {
      BrewTargetTreeItem* local = rootItem->child(BrewTargetTreeItem::YEAST);
      QList<Yeast*>::iterator it;

      rows = dbObs->getNumYeasts();
      insertRows(0,rows,createIndex(BrewTargetTreeItem::YEAST,0,local));

      for( i = 0, it = dbObs->getYeastBegin(); it != dbObs->getYeastEnd(); ++it, ++i )
      {
        BrewTargetTreeItem* temp = local->child(i);
        temp->setData(BrewTargetTreeItem::YEAST,*it);
      }
   }
}

void BrewTargetTreeModel::unloadTreeModel(int unload)
{
   int breadth;
   QModelIndex parent;

   if ( unload == DBALL || unload == DBRECIPE)
   {
      removeAllObserved();
      parent = createIndex(BrewTargetTreeItem::RECIPE,0,rootItem->child(BrewTargetTreeItem::RECIPE));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if (unload == DBALL || unload == DBEQUIP)
   {
      parent = createIndex(BrewTargetTreeItem::EQUIPMENT,0,rootItem->child(BrewTargetTreeItem::EQUIPMENT));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if (unload == DBALL || unload == DBFERM)
   {
      parent = createIndex(BrewTargetTreeItem::FERMENTABLE,0,rootItem->child(BrewTargetTreeItem::FERMENTABLE));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }

   if (unload == DBALL || unload == DBHOP)
   {
      parent = createIndex(BrewTargetTreeItem::HOP,0,rootItem->child(BrewTargetTreeItem::HOP));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if (unload == DBALL || unload == DBMISC)
   {
      parent = createIndex(BrewTargetTreeItem::MISC,0,rootItem->child(BrewTargetTreeItem::MISC));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
   if (unload == DBALL || unload == DBYEAST)
   {
      parent = createIndex(BrewTargetTreeItem::YEAST,0,rootItem->child(BrewTargetTreeItem::YEAST));
      breadth = rowCount(parent);
      removeRows(0,breadth,parent);
   }
}

void BrewTargetTreeModel::notify(Observable* notifier, QVariant info)
{

   // Notifier could be the database. 
   if( notifier == dbObs )
   {
     unloadTreeModel(info.toInt());
     loadTreeModel(info.toInt());
   }
   else // Otherwise, we know that one of the recipes changed.
   {
      Recipe* foo = static_cast<Recipe*>(notifier);
      QModelIndex changed = findRecipe(foo);

      if ( ! changed.isValid() )
         return;

      BrewTargetTreeItem* temp = getItem(changed);
      temp->setData(BrewTargetTreeItem::RECIPE,foo);

      // If the tree item has children -- that is, brew notes -- remove them
      // all
      if ( temp->childCount() )
         removeRows(0,temp->childCount(),changed);
      
      if ( foo->getNumBrewNotes() > 0 ) {

         // Put back how many ever rows are left
         insertRows(0,foo->getNumBrewNotes(),changed);
         for (unsigned int j=0; j < foo->getNumBrewNotes(); ++j)
         {
            // And populate them.
            BrewTargetTreeItem* bar = temp->child(j);
            bar->setData(BrewTargetTreeItem::BREWNOTE,foo->getBrewNote(j));
         }
      }
   }
}

Recipe* BrewTargetTreeModel::getRecipe(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getRecipe();
}

Equipment* BrewTargetTreeModel::getEquipment(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getEquipment();
}

Fermentable* BrewTargetTreeModel::getFermentable(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getFermentable();
}

Hop* BrewTargetTreeModel::getHop(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getHop();
}

Misc* BrewTargetTreeModel::getMisc(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getMisc();
}

Yeast* BrewTargetTreeModel::getYeast(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getYeast();
}

BrewNote* BrewTargetTreeModel::getBrewNote(const QModelIndex &index) const
{
   BrewTargetTreeItem* item = getItem(index);

   return item->getBrewNote();
}

bool BrewTargetTreeModel::isRecipe(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::RECIPE;
}

bool BrewTargetTreeModel::isEquipment(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::EQUIPMENT;
}

bool BrewTargetTreeModel::isFermentable(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::FERMENTABLE;
}

bool BrewTargetTreeModel::isHop(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::HOP;
}

bool BrewTargetTreeModel::isMisc(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::MISC;
}

bool BrewTargetTreeModel::isYeast(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::YEAST;
}

bool BrewTargetTreeModel::isBrewNote(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType() == BrewTargetTreeItem::BREWNOTE;
}

int BrewTargetTreeModel::getType(const QModelIndex &index)
{
   BrewTargetTreeItem* item = getItem(index);
   return item->getType();
}