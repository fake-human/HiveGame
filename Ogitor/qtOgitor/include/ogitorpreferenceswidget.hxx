/*/////////////////////////////////////////////////////////////////////////////////
/// An
///    ___   ____ ___ _____ ___  ____
///   / _ \ / ___|_ _|_   _/ _ \|  _ \
///  | | | | |  _ | |  | || | | | |_) |
///  | |_| | |_| || |  | || |_| |  _ <
///   \___/ \____|___| |_| \___/|_| \_\
///                              File
///
/// Copyright (c) 2008-2011 Ismail TARIM <ismail@royalspor.com> and the Ogitor Team
//
/// The MIT License
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////*/

#ifndef OGITORPREFERENCESWIDGET_HXX
#define OGITORPREFERENCESWIDGET_HXX

#include <QtGui/QWidget>
#include <QtGui/QTreeWidget>
#include <QtCore/QMap>
#include "OgitorsPrerequisites.h"
#include "OgitorsRoot.h"

#include "ui_ogitorpreferencestab.h"



class OgitorPreferencesWidget : public QWidget, Ui::ogitorPreferencesWidget
{
    Q_OBJECT
public:
    OgitorPreferencesWidget(QWidget *parent = 0);
    virtual ~OgitorPreferencesWidget();

    void     getPreferences(Ogre::NameValuePairList& preferences);
    void    *getPreferencesWidget(const Ogre::NameValuePairList& list);
    bool     applyPreferences();
    
    // Methods for easy access to preference values
    QString  getPrefCustomStyleSheet();
    QString  getPrefCustomLanguage();

public Q_SLOTS:
    void setDirty();
    void languageChanged();
    
Q_SIGNALS:
    void isDirty();

private:
    QTreeWidget *treeWidget;

    void fillOgreTab();
    
    bool                    mLanguageChanged;
    QMap<QString, QString>  mLanguageMap;
};

#endif // OGITORPREFERENCESWIDGET_HXX
