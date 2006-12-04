/** Copyright (C) 2006, Ian Paul Larsen.
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation; either version 2 of the License, or
 **  (at your option) any later version.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License along
 **  with this program; if not, write to the Free Software Foundation, Inc.,
 **  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

using namespace std;
#include <iostream>

#include "VariableWin.h"
#include <QHeaderView>

VariableWin::VariableWin (QWidget * parent) 
  :QDockWidget(QString(tr("Variable Watch")), parent)
{
  rows = 0;
  table = new QTreeWidget(this);
  table->setColumnCount(2);
  setWidget(table);
}


void
VariableWin::addVar(QString name, QString value)
{
  QTreeWidgetItem *rowItem = new QTreeWidgetItem();
  rowItem->setText(0, name);
  rowItem->setText(1, value);
  for (unsigned int i = 0; i < rows; i++)
    {
      QTreeWidgetItem *item = table->topLevelItem(i);
      if (item->text(0) == name)
	{
	  item->setText(1, value);
	  return;
	}
    }
 
  table->insertTopLevelItem(rows, rowItem);
  rows++;
}

void
VariableWin::clearTable()
{
  table->clear();
  rows = 0;
}
