//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "preferenceslistwidget.h"
#include <cfloat>

namespace Ms {


PreferencesListWidget::PreferencesListWidget(QWidget* parent)
      : QTreeWidget(parent)
      {
      setRootIsDecorated(false);
      setHeaderLabels(QStringList() << tr("Preference") << tr("Value"));
      header()->setStretchLastSection(false);
      header()->setSectionResizeMode(0, QHeaderView::Stretch);
      setAccessibleName(tr("Advanced preferences"));
      setAccessibleDescription(tr("Access to more advanced preferences"));
      setAlternatingRowColors(true);
      setSortingEnabled(true);
      sortByColumn(0, Qt::AscendingOrder);
      setAllColumnsShowFocus(true);
      }

void PreferencesListWidget::loadPreferences()
      {
      for (QString key : preferences.allPreferences().keys()) {
            Preference* pref = preferences.allPreferences().value(key);

            if (pref->showInAdvancedList()) {
                  // multiple dispatch using Visitor pattern, see overloaded visit() methods
                  pref->accept(key, *this);
                  }
            }
      }

void PreferencesListWidget::updatePreferences()
      {
      for (PreferenceItem* item : preferenceItems.values())
            item->update();
      }

void PreferencesListWidget::addPreference(PreferenceItem* item)
      {
      addTopLevelItem(item);
      setItemWidget(item, PREF_VALUE_COLUMN, item->editor());
      preferenceItems[item->name()] = item;
      }

void PreferencesListWidget::visit(QString key, IntPreference*)
      {
      IntPreferenceItem* item = new IntPreferenceItem(key);
      addPreference(item);
      }

void PreferencesListWidget::visit(QString key, DoublePreference*)
      {
      DoublePreferenceItem* item = new DoublePreferenceItem(key);
      addPreference(item);
      }

void PreferencesListWidget::visit(QString key, BoolPreference*)
      {
      BoolPreferenceItem* item = new BoolPreferenceItem(key);
      addPreference(item);
      }

void PreferencesListWidget::visit(QString key, StringPreference*)
      {
      StringPreferenceItem* item = new StringPreferenceItem(key);
      addPreference(item);
      }

void PreferencesListWidget::visit(QString key, ColorPreference*)
      {
      ColorPreferenceItem* item = new ColorPreferenceItem(key);
      addPreference(item);
      }

std::vector<QString> PreferencesListWidget::save()
      {
      std::vector<QString> changedPreferences;
      for (int i = 0; i < topLevelItemCount(); ++i) {
            PreferenceItem* item = static_cast<PreferenceItem*>(topLevelItem(i));
            if (item->isModified()) {
                  item->save();
                  changedPreferences.push_back(item->name());
                  }
            }

      return changedPreferences;
      }

//---------------------------------------------------------
//   PreferenceItem
//---------------------------------------------------------

PreferenceItem::PreferenceItem()
      {
      }

PreferenceItem::PreferenceItem(QString name)
      : _name(name)
      {
      setText(0, name);
      setSizeHint(0, QSize(0, 25));
      }

void PreferenceItem::save(QVariant value)
      {
      preferences.setPreference(name(), value);
      }

//---------------------------------------------------------
//   ColorPreferenceItem
//---------------------------------------------------------

ColorPreferenceItem::ColorPreferenceItem(QString name)
      : PreferenceItem(name),
        _initialValue(preferences.getColor(name)),
        _editor(new Awl::ColorLabel)
      {
      _editor->setColor(_initialValue);
      _editor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
      connect(_editor, &Awl::ColorLabel::colorChanged, this, &PreferenceItem::editorValueModified);
      }

ColorPreferenceItem::ColorPreferenceItem(QString name, Awl::ColorLabel* editor)
      : PreferenceItem(name),
        _initialValue(preferences.getColor(name)),
        _editor(editor)
      {
      _editor->setColor(_initialValue);
      _editor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
      connect(_editor, &Awl::ColorLabel::colorChanged, this, &PreferenceItem::editorValueModified);
      }

void ColorPreferenceItem::save()
      {
      QColor newValue = _editor->color();
      _initialValue = newValue;
      PreferenceItem::save(newValue);
      }

void ColorPreferenceItem::update()
      {
      QColor newValue = preferences.getColor(name());
      _editor->setColor(newValue);
      }

void ColorPreferenceItem::setDefaultValue()
      {
      _editor->setColor(preferences.defaultValue(name()).value<QColor>());
      }

QWidget* ColorPreferenceItem::editor() const
      {
      return _editor;
      }

bool ColorPreferenceItem::isModified() const
      {
      return _initialValue != _editor->color();
      }


//---------------------------------------------------------
//   IntPreferenceItem
//---------------------------------------------------------

IntPreferenceItem::IntPreferenceItem(QString name)
      : PreferenceItem(name),
        _initialValue(preferences.getInt(name)),
        _editor(new QSpinBox)
      {
      _editor->setMaximum(INT_MAX);
      _editor->setMinimum(INT_MIN);
      _editor->setValue(_initialValue);
      connect(_editor, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferenceItem::editorValueModified);
      }

IntPreferenceItem::IntPreferenceItem(QString name, QSpinBox* editor)
      : PreferenceItem(name),
        _initialValue(preferences.getInt(name)),
        _editor(editor)
      {
      _editor->setMaximum(INT_MAX);
      _editor->setMinimum(INT_MIN);
      _editor->setValue(_initialValue);
      connect(_editor, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferenceItem::editorValueModified);
      }

IntPreferenceItem::IntPreferenceItem(QString name, QComboBox* editor)
      : PreferenceItem(name),
        _initialValue(preferences.getInt(name)),
        _editor2(editor)
      {
      int index = _editor2->findData(preferences.getInt(name));
      _editor2->setCurrentIndex(index);
      connect(_editor2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceItem::editorValueModified);
      }

void IntPreferenceItem::save()
      {
      if (_editor) {
            int newValue = _editor->value();
            _initialValue = newValue;
            PreferenceItem::save(newValue);
            }
      else if (_editor2) {
            int newValue = _editor2->currentData().toInt();
            _initialValue = newValue;
            PreferenceItem::save(newValue);
            }
      }

void IntPreferenceItem::update()
      {
      if (_editor) {
            int newValue = preferences.getInt(name());
            _editor->setValue(newValue);
            }
      else if (_editor2) {
            int index = _editor2->findData(preferences.getInt(name()));
            if (index == -1)
                  setDefaultValue();
            else
                  _editor2->setCurrentIndex(index);
            }
      }

void IntPreferenceItem::setDefaultValue()
      {
      if (_editor) {
            _editor->setValue(preferences.defaultValue(name()).toInt());
            }
      else if (_editor2) {
            int index = _editor2->findData(preferences.defaultValue(name()).toInt());
            Q_ASSERT(index != -1);
            _editor2->setCurrentIndex(index);
            }
      }

QWidget* IntPreferenceItem::editor() const
      {
      if (_editor)
            return _editor;
      else if (_editor2)
            return _editor2;
      else
            return nullptr;
      }


bool IntPreferenceItem::isModified() const
      {
      if (_editor)
            return _initialValue != _editor->value();
      else if (_editor2)
            return _initialValue != _editor2->currentData().toInt();
      else
            Q_ASSERT(false);
      }

//---------------------------------------------------------
//   DoublePreferenceItem
//---------------------------------------------------------

DoublePreferenceItem::DoublePreferenceItem(QString name)
      : PreferenceItem(name),
        _initialValue(preferences.getDouble(name)),
        _editor(new QDoubleSpinBox)
      {
      _editor->setMaximum(DBL_MAX);
      _editor->setMinimum(DBL_MIN);
      _editor->setValue(_initialValue);
      if (qAbs(_initialValue) < 2.0)
            _editor->setSingleStep(0.1);
      connect(_editor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PreferenceItem::editorValueModified);
      }

DoublePreferenceItem::DoublePreferenceItem(QString name, QDoubleSpinBox* editor, double modifier)
      : PreferenceItem(name),
        _initialValue(preferences.getDouble(name)),
        _modifier(modifier),
        _editor(editor)
      {
      _editor->setMaximum(DBL_MAX);
      _editor->setMinimum(DBL_MIN);
      _editor->setValue(_initialValue * modifier);
      if (qAbs(_initialValue) < 2.0)
            _editor->setSingleStep(0.1);
      connect(_editor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PreferenceItem::editorValueModified);
      }

DoublePreferenceItem::DoublePreferenceItem(QString name, QComboBox* editor, double modifier)
      : PreferenceItem(name),
        _initialValue(preferences.getDouble(name)),
        _modifier(modifier),
        _editor2(editor)
      {
      int index = _editor2->findData(preferences.getDouble(name));
      _editor2->setCurrentIndex(index);
      connect(_editor2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceItem::editorValueModified);
      }

void DoublePreferenceItem::save()
      {
      if (_editor) {
            double newValue = _editor->value() / _modifier;
            _initialValue = newValue;
            PreferenceItem::save(newValue);
            }
      else if (_editor2) {
            double newValue = _editor2->currentData().toDouble();
            _initialValue = newValue;
            PreferenceItem::save(newValue);
            }
      }

void DoublePreferenceItem::update()
      {
      if (_editor) {
            double newValue = preferences.getDouble(name()) * _modifier;
            _editor->setValue(newValue);
            }
      else if (_editor2) {
            int index = _editor2->findData(preferences.getDouble(name()));
            if (index == -1)
                  setDefaultValue();
            else
                  _editor2->setCurrentIndex(index);
            }
      }

void DoublePreferenceItem::setDefaultValue()
      {
      if (_editor){
            _editor->setValue(preferences.defaultValue(name()).toDouble() * _modifier);
            }
      else if (_editor2) {
            int index = _editor2->findData(preferences.defaultValue(name()).toDouble());
            Q_ASSERT(index != -1);
            _editor2->setCurrentIndex(index);
            }
      }

QWidget* DoublePreferenceItem::editor() const
      {
      if (_editor)
            return _editor;
      else if (_editor2)
            return _editor2;
      else
            Q_ASSERT(false);
      }

bool DoublePreferenceItem::isModified() const
      {
      if (_editor)
            return _initialValue * _modifier != _editor->value();
      else if (_editor2)
            return _initialValue != _editor2->currentData().toDouble();
      else
            Q_ASSERT(false);
      }


//---------------------------------------------------------
//   BoolPreferenceItem
//---------------------------------------------------------

BoolPreferenceItem::BoolPreferenceItem(QString name)
      : PreferenceItem(name),
        _initialValue(preferences.getBool(name)),
        _editor(new QCheckBox)
      {
      _editor->setChecked(_initialValue);
      connect(_editor, &QCheckBox::toggled, this, &PreferenceItem::editorValueModified);
      }

BoolPreferenceItem::BoolPreferenceItem(QString name, QCheckBox* editor)
      : PreferenceItem(name),
        _initialValue(preferences.getBool(name)),
        _editor(editor)
      {
      _editor->setChecked(_initialValue);
      connect(_editor, &QCheckBox::toggled, this, &PreferenceItem::editorValueModified);
      }

BoolPreferenceItem::BoolPreferenceItem(QString name, QGroupBox* editor)
      : PreferenceItem(name),
        _initialValue(preferences.getBool(name)),
        _editor2(editor)
      {
      _editor2->setChecked(_initialValue);
      connect(_editor2, &QGroupBox::toggled, this, &PreferenceItem::editorValueModified);
      }

BoolPreferenceItem::BoolPreferenceItem(QString name, QRadioButton* editor)
      : PreferenceItem(name),
        _initialValue(preferences.getBool(name)),
        _editor3(editor)
      {
      _editor3->setChecked(_initialValue);
      connect(_editor3, &QRadioButton::toggled, this, &PreferenceItem::editorValueModified);
      }

void BoolPreferenceItem::save()
      {
      if (_editor) {
            bool newValue = _editor->isChecked();
            _initialValue = newValue;
            PreferenceItem::save(newValue);
            }
      else if (_editor2) {
            bool newValue = _editor2->isChecked();
            _initialValue = newValue;
            PreferenceItem::save(newValue);
            }
      else if (_editor3) {
            bool newValue = _editor3->isChecked();
            _initialValue = newValue;
            PreferenceItem::save(newValue);
            }
      }

void BoolPreferenceItem::update()
      {
      if (_editor) {
            bool newValue = preferences.getBool(name());
            _editor->setChecked(newValue);
            }
      else if (_editor2) {
            bool newValue = preferences.getBool(name());
            _editor2->setChecked(newValue);
            }
      else if (_editor3) {
            bool newValue = preferences.getBool(name());
            _editor3->setChecked(newValue);
            }
      }

void BoolPreferenceItem::setDefaultValue()
      {
      if (_editor)
            _editor->setChecked(preferences.defaultValue(name()).toBool());
      else if (_editor2)
            _editor2->setChecked(preferences.defaultValue(name()).toBool());
      else if (_editor3)
            _editor3->setChecked(preferences.defaultValue(name()).toBool());
      }

QWidget* BoolPreferenceItem::editor() const
      {
      if (_editor)
            return _editor;
      else if (_editor2)
            return _editor2;
      else if (_editor3)
            return _editor3;
      else
            return nullptr;
      }

bool BoolPreferenceItem::isModified() const
      {
      if (_editor)
            return _initialValue != _editor->isChecked();
      else if (_editor2)
            return _initialValue != _editor2->isChecked();
      else if (_editor3)
            return _initialValue != _editor3->isChecked();
      else
            Q_ASSERT(false);
      }

//---------------------------------------------------------
//   StringPreferenceItem
//---------------------------------------------------------

StringPreferenceItem::StringPreferenceItem(QString name, std::function<void()> func)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editor(new QLineEdit)
      {
      _editor->setText(_initialValue);
      connect(_editor, &QLineEdit::textChanged, this, &PreferenceItem::editorValueModified);
      function = func;
      }

StringPreferenceItem::StringPreferenceItem(QString name, QLineEdit* editor, std::function<void()> func)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editor(editor)
      {
      _editor->setText(_initialValue);
      connect(_editor, &QLineEdit::textChanged, this, &PreferenceItem::editorValueModified);     
      function = func;
      }

StringPreferenceItem::StringPreferenceItem(QString name, QFontComboBox* editor, std::function<void()> func)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editor2(editor)
      {
      _editor2->setCurrentFont(QFont(_initialValue));
      connect(_editor2, &QFontComboBox::currentFontChanged, this, &PreferenceItem::editorValueModified);
      function = func;
      }

StringPreferenceItem::StringPreferenceItem(QString name, QComboBox* editor, std::function<void()> func)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editor3(editor)
      {
      int index = _editor3->findData(preferences.getString(name));
      _editor3->setCurrentIndex(index);
      connect(_editor3, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceItem::editorValueModified);
      function = func;
      }


void StringPreferenceItem::save()
      {
      if (_editor) {
            QString newValue = _editor->text();
            _initialValue = newValue;
            PreferenceItem::save(newValue);
            }
      else if (_editor2) {
            QString newValue = _editor2->currentFont().family();
            _initialValue = newValue;
            PreferenceItem::save(newValue);
            }
      else if (_editor3) {
            QString newValue = _editor3->currentText();
            _initialValue = newValue;
            PreferenceItem::save(newValue);
            }
      if (function.operator bool())
            function();
      }

void StringPreferenceItem::update()
      {
      if (_editor) {
            QString newValue = preferences.getString(name());
            _editor->setText(newValue);
            }
      else if (_editor2) {
            QString newValue = preferences.getString(name());
            _editor2->setCurrentFont(QFont(newValue));
            }
      else if (_editor3) {
            int index = _editor3->findData(preferences.getString(name()));
            if (index == -1)
                  setDefaultValue();
            else
                  _editor3->setCurrentIndex(index);
            }
      }

void StringPreferenceItem::setDefaultValue()
      {
      if (_editor) {
            _editor->setText(preferences.defaultValue(name()).toString());
            }
      else if (_editor2) {
            _editor2->setCurrentFont(QFont(preferences.defaultValue(name()).toString()));
            }
      else if (_editor3) {
            int index = _editor3->findData(preferences.defaultValue(name()).toString());
            Q_ASSERT(index != -1);
            _editor3->setCurrentIndex(index);
            }
      if (function.operator bool())
            function();
      }

QWidget* StringPreferenceItem::editor() const
      {
      if (_editor)
            return _editor;
      else if (_editor2)
            return _editor2;
      else if (_editor3)
            return _editor3;
      else
            return nullptr;
      }

bool StringPreferenceItem::isModified() const
      {
      if (_editor)
            return _initialValue != _editor->text();
      else if (_editor2)
            return _initialValue != _editor2->currentFont().family();
      else if (_editor3)
            return _initialValue != _editor3->currentText(); // this should be currentData.toString() but this causes a crash INVESTIGATE!
      else
            Q_ASSERT(false);
      }



} // namespace Ms
