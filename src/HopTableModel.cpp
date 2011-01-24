/*
 * HopTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QComboBox>
#include <QLineEdit>

#include "hop.h"
#include <QString>
#include <QVector>
#include <iostream>
#include "hop.h"
#include "observable.h"
#include "HopTableModel.h"
#include "unit.h"
#include "brewtarget.h"

HopTableModel::HopTableModel(HopTableWidget* parent)
: QAbstractTableModel(parent), MultipleObserver()
{
   hopObs.clear();
   parentTableWidget = parent;
   showIBUs = false;
   recObs = 0;
}

void HopTableModel::addHop(Hop* hop)
{
   QVector<Hop*>::iterator iter;
   
   //Check to see if it's already in the list
   for( iter=hopObs.begin(); iter != hopObs.end(); iter++ )
      if( *iter == hop )
         return;
   
   hopObs.push_back(hop);
   addObserved(hop);
   reset(); // Tell everybody that the table has changed.

   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

void HopTableModel::setShowIBUs( bool var )
{
   showIBUs = var;
}

void HopTableModel::setRecipe( Recipe* rec )
{
   if( recObs )
      removeObserved(recObs);
   
   if( rec )
   {
      addObserved(rec);
      recObs = rec;
   }
}

bool HopTableModel::removeHop(Hop* hop)
{
   QVector<Hop*>::iterator iter;
   
   for( iter=hopObs.begin(); iter != hopObs.end(); iter++ )
      if( *iter == hop )
      {
         hopObs.erase(iter);
         removeObserved(hop);
         reset(); // Tell everybody the table has changed.
         
         if( parentTableWidget )
         {
            parentTableWidget->resizeColumnsToContents();
            parentTableWidget->resizeRowsToContents();
         }
         
         return true;
      }
   
   return false;
}

void HopTableModel::removeAll()
{
   int i;

   for( i = 0; i < hopObs.size(); ++i )
      removeObserved(hopObs[i]);

   hopObs.clear();
   reset();
}

void HopTableModel::notify(Observable* notifier, QVariant info)
{
   int i;
   
   if( notifier == recObs )
      emit headerDataChanged( Qt::Vertical, 0, rowCount()-1 );
   
   // Find the notifier in the list
   for( i = 0; i < (int)hopObs.size(); ++i )
   {
      if( notifier == hopObs[i] )
         emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                           QAbstractItemModel::createIndex(i, HOPNUMCOLS));
   }
}

int HopTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return hopObs.size();
}

int HopTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return HOPNUMCOLS;
}

QVariant HopTableModel::data( const QModelIndex& index, int role ) const
{
   Hop* row;
   
   // Ensure the row is ok.
   if( index.row() >= (int)hopObs.size() )
   {
      Brewtarget::log(Brewtarget::WARNING, tr("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = hopObs[index.row()];

   switch( index.column() )
   {
      case HOPNAMECOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->getName());
         else
            return QVariant();
      case HOPALPHACOL:
         if( role == Qt::DisplayRole )
            return QVariant( Brewtarget::displayAmount(row->getAlpha_pct(), 0) );
         else
            return QVariant();
      case HOPAMOUNTCOL:
         if( role == Qt::DisplayRole )
            return QVariant( Brewtarget::displayAmount(row->getAmount_kg(), Units::kilograms) );
         else
            return QVariant();
      case HOPUSECOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->getUseString());
         else if( role == Qt::UserRole )
            return QVariant(row->getUse());
         else
            return QVariant();
      case HOPTIMECOL:
         if( role == Qt::DisplayRole )
            return QVariant( Brewtarget::displayAmount(row->getTime_min(), Units::minutes) );
         else
            return QVariant();
      case HOPFORMCOL:
    	 if ( role == Qt::DisplayRole )
    		 return QVariant( row->getFormString());
    	 else if ( role == Qt::UserRole )
    		 return QVariant( row->getForm());
    	 else
    		 return QVariant();
      default :
         Brewtarget::log(Brewtarget::WARNING, tr("HopTableModel::data Bad column: %1").arg(index.column()));
         return QVariant();
   }
}

QVariant HopTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case HOPNAMECOL:
            return QVariant(tr("Name"));
         case HOPALPHACOL:
            return QVariant(tr("Alpha %"));
         case HOPAMOUNTCOL:
            return QVariant(tr("Amount"));
         case HOPUSECOL:
            return QVariant(tr("Use"));
         case HOPTIMECOL:
            return QVariant(tr("Time"));
         case HOPFORMCOL:
        	return QVariant(tr("Form"));
         default:
            Brewtarget::log(Brewtarget::WARNING, tr("HopTableModel::headerdata Bad column: %1").arg(section));
            return QVariant();
      }
   }
   else if( showIBUs && recObs && orientation == Qt::Vertical && role == Qt::DisplayRole )
   {
      double ibus = recObs->getIBUFromHop( section );
      return QVariant( QString("%1 IBU").arg( ibus, 0, 'f', 1 ) );
   }
   else
      return QVariant();
}

Qt::ItemFlags HopTableModel::flags(const QModelIndex& index ) const
{
   int col = index.column();
   
   if( col == HOPNAMECOL )
      return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   else
      return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
         Qt::ItemIsEnabled;
}

bool HopTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Hop *row;
   
   if( index.row() >= (int)hopObs.size() || role != Qt::EditRole )
      return false;
   else
      row = hopObs[index.row()];
   
   switch( index.column() )
   {
      case HOPNAMECOL:
         if( value.canConvert(QVariant::String))
         {
            row->setName(value.toString());
            return true;
         }
         else
            return false;
      case HOPALPHACOL:
         if( value.canConvert(QVariant::Double) )
         {
            row->setAlpha_pct( value.toDouble() );
            headerDataChanged( Qt::Vertical, index.row(), index.row() ); // Need to re-show header (IBUs).
            return true;
         }
         else
            return false;
      case HOPAMOUNTCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setAmount_kg( Brewtarget::weightQStringToSI(value.toString()) );
            headerDataChanged( Qt::Vertical, index.row(), index.row() ); // Need to re-show header (IBUs).
            return true;
         }
         else
            return false;
      case HOPUSECOL:
         if( value.canConvert(QVariant::Int) )
         {
            row->setUse(static_cast<Hop::Use>(value.toInt()));
            headerDataChanged( Qt::Vertical, index.row(), index.row() ); // Need to re-show header (IBUs).
            return true;
         }
         else
            return false;
      case HOPFORMCOL:
    	  if( value.canConvert(QVariant::Int))
    	  {
    		  row->setForm(static_cast<Hop::Form>(value.toInt()));
    		  headerDataChanged( Qt::Vertical, index.row(), index.row() );
    		  return true;
    	  }
      case HOPTIMECOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setTime_min( Brewtarget::timeQStringToSI(value.toString()) );
            headerDataChanged( Qt::Vertical, index.row(), index.row() ); // Need to re-show header (IBUs).
            return true;
         }
         else
            return false;
      default:
         Brewtarget::log(Brewtarget::WARNING, QString("HopTableModel::setdata Bad column: %1").arg(index.column()));
         return false;
   }
}

Hop* HopTableModel::getHop(unsigned int i)
{
   return hopObs[i];
}

//==========================CLASS HopItemDelegate===============================

HopItemDelegate::HopItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}
        
QWidget* HopItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex &index) const
{
	if ( index.column() == HOPUSECOL )
	{
		QComboBox *box = new QComboBox(parent);

		box->addItem(tr("Boil"));
		box->addItem(tr("Dry Hop"));
		box->addItem(tr("Mash"));
		box->addItem(tr("First Wort"));
		box->addItem(tr("Aroma"));
		box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

		return box;
	}
	else if ( index.column() == HOPFORMCOL )
	{
		QComboBox *box = new QComboBox(parent);

		box->addItem(tr("Leaf"));
		box->addItem(tr("Pellet"));
		box->addItem(tr("Plug"));


		box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

		return box;
	}
	else
	{
		return new QLineEdit(parent);
	}
}

void HopItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	if (index.column() == HOPUSECOL )
	{
		QComboBox* box = (QComboBox*)editor;
		int ndx = index.model()->data(index, Qt::UserRole).toInt();

		box->setCurrentIndex(ndx);
	}
	else if ( index.column() == HOPFORMCOL )
	{
		QComboBox* box = (QComboBox*)editor;
		int ndx = index.model()->data(index,Qt::UserRole).toInt();

		box->setCurrentIndex(ndx);
	}
	else
	{
	    QLineEdit* line = (QLineEdit*)editor;
	    line->setText(index.model()->data(index, Qt::DisplayRole).toString());
	}
}

void HopItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	if ( index.column() == HOPUSECOL )
	{
		QComboBox* box = (QComboBox*)editor;
	    int value = box->currentIndex();
	    model->setData(index, value, Qt::EditRole);
	}
	else if (index.column() == HOPFORMCOL )
	{
	    QComboBox* box = (QComboBox*)editor;
	    int value = box->currentIndex();
	    model->setData(index, value, Qt::EditRole);
	}
	else
	{
	 	QLineEdit* line = (QLineEdit*)editor;
		model->setData(index, line->text(), Qt::EditRole);
    }
}

void HopItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}
