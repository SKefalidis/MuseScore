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

#ifndef __PREFERENCESLISTWIDGET_H__
#define __PREFERENCESLISTWIDGET_H__

#include "awl/colorlabel.h"
#include "preferences.h"

#define PREF_VALUE_COLUMN 1

namespace Ms {

//---------------------------------------------------------
//   PreferenceItem
//---------------------------------------------------------
class PreferenceItem : public QObject, public QTreeWidgetItem {

      Q_OBJECT

      QString _name;

    protected:
      void save(QVariant value);

    public:
      PreferenceItem();
      PreferenceItem(QString name);

      virtual void save() = 0;
      virtual void update() = 0;
      virtual void setDefaultValue() = 0;
      virtual QWidget* editor() const = 0;
      virtual bool isModified() const = 0;

      QString name() const {return _name;}

   signals:
      void editorValueModified();
      };

//---------------------------------------------------------
//   BoolPreferenceItem
//---------------------------------------------------------
class BoolPreferenceItem : public PreferenceItem {
      bool _initialValue;
      QCheckBox* _editor      {nullptr}; // make a QVariant out of these 3
      QGroupBox* _editor2     {nullptr};
      QRadioButton* _editor3  {nullptr};
      std::function<void()> applyFunction = {};
      std::function<void()> updateFunction = {};

   public:
      BoolPreferenceItem(QString name, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      BoolPreferenceItem(QString name, QCheckBox* editor, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      BoolPreferenceItem(QString name, QGroupBox* editor, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      BoolPreferenceItem(QString name, QRadioButton* editor, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});

      void save() override;
      void update() override;
      void setDefaultValue() override;
      QWidget* editor() const override;
      bool isModified() const override;
      };


//---------------------------------------------------------
//   IntPreferenceItem
//---------------------------------------------------------
class IntPreferenceItem : public PreferenceItem {
      int _initialValue;
      QSpinBox* _editor { nullptr };
      QComboBox* _editor2 { nullptr };
      std::function<void()> applyFunction = {};
      std::function<void()> updateFunction = {};

   public:
      IntPreferenceItem(QString name, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      IntPreferenceItem(QString name, QSpinBox* editor, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      IntPreferenceItem(QString name, QComboBox* editor, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});

      void save() override;
      void update() override;
      void setDefaultValue() override;
      QWidget* editor() const override;
      bool isModified() const override;
      };

//---------------------------------------------------------
//   DoublePreferenceItem
//---------------------------------------------------------
class DoublePreferenceItem : public PreferenceItem {
      double _initialValue { 0 }, _modifier { 1 };
      QDoubleSpinBox* _editor { nullptr };
      QComboBox* _editor2 { nullptr };
      std::function<void()> applyFunction = {};
      std::function<void()> updateFunction = {};

   public:
      DoublePreferenceItem(QString name, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      DoublePreferenceItem(QString name, QDoubleSpinBox* editor, double modifier = 1, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      DoublePreferenceItem(QString name, QComboBox* editor, double modifier = 1, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});

      void save() override;
      void update() override;
      void setDefaultValue() override;
      QWidget* editor() const override;
      bool isModified() const override;
      };

//---------------------------------------------------------
//   StringPreferenceItem
//---------------------------------------------------------
class StringPreferenceItem : public PreferenceItem {
      QString _initialValue;
      QLineEdit* _editor { nullptr };
      QFontComboBox* _editor2 { nullptr };
      QComboBox* _editor3 { nullptr };
      std::function<void()> applyFunction = {};
      std::function<void()> updateFunction = {};

   public:
      StringPreferenceItem(QString name, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      StringPreferenceItem(QString name, QLineEdit* editor, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      StringPreferenceItem(QString name, QFontComboBox* editor, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      StringPreferenceItem(QString name, QComboBox* editor, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});

      void save() override;
      void update() override;
      void setDefaultValue() override;
      QWidget* editor() const override;
      bool isModified() const override;
      };

//---------------------------------------------------------
//   ColorPreferenceItem
//---------------------------------------------------------
class ColorPreferenceItem : public PreferenceItem {
      QColor _initialValue;
      Awl::ColorLabel* _editor { nullptr };
      std::function<void()> applyFunction = {};
      std::function<void()> updateFunction = {};

   public:
      ColorPreferenceItem(QString name, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});
      ColorPreferenceItem(QString name, Awl::ColorLabel* editor, std::function<void()> applyFunc = {}, std::function<void()> updateFunc = {});

      void save() override;
      void update() override;
      void setDefaultValue() override;
      QWidget* editor() const override;
      bool isModified() const override;
      };


//---------------------------------------------------------
//   PreferencesListWidget
//---------------------------------------------------------

class PreferencesListWidget : public QTreeWidget, public PreferenceVisitor {
      Q_OBJECT

      QHash<QString, PreferenceItem*> preferenceItems;

      void addPreference(PreferenceItem* item);

   public:
      explicit PreferencesListWidget(QWidget* parent = 0);
      void loadPreferences();
      void updatePreferences();

      std::vector<QString> save();

      void visit(QString key, IntPreference*);
      void visit(QString key, DoublePreference*);
      void visit(QString key, BoolPreference*);
      void visit(QString key, StringPreference*);
      void visit(QString key, ColorPreference*);
      };

} // namespace Ms

#endif // __PREFERENCESLISTWIDGET_H__
