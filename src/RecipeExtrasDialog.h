/*
* RecipeExtrasDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2010.
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

#ifndef RECIPEEXTRASDIALOG_H
#define RECIPEEXTRASDIALOG_H

class RecipeExtrasDialog;

#include <QDialog>
#include <QWidget>
#include "ui_recipeExtrasDialog.h"
#include "observable.h"
#include "recipe.h"

class RecipeExtrasDialog : public QDialog, public Ui::recipeExtrasDialog, public Observer
{
   Q_OBJECT

public:
   RecipeExtrasDialog(QWidget* parent=0);
   void setRecipe(Recipe* rec);

public slots:
   void updateBrewer();
   void updateBrewerAsst();
   void updateTasteRating();
   void updatePrimaryAge();
   void updatePrimaryTemp();
   void updateSecondaryAge();
   void updateSecondaryTemp();
   void updateTertiaryAge();
   void updateTertiaryTemp();
   void updateAge();
   void updateAgeTemp();
   void updateDate();
   void updateCarbonation();
   void updateTasteNotes();
   void updateNotes();

   void saveAndQuit();

private:
   Recipe* recObs;

   void showChanges();

   virtual void notify(Observable* notifier, QVariant info = QVariant()); // Inherited from Observer
};

#endif // RECIPEEXTRASDIALOG_H