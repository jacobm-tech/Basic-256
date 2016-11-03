/** Copyright (C) 2010, J.M.Reneau
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

#include <iostream>



#include "PreferencesWin.h"
#include "Settings.h"
#include "MainWindow.h"
#include "md5.h"

#ifdef ESPEAK
#include <speak_lib.h>
#endif

PreferencesWin::PreferencesWin (QWidget * parent, bool showAdvanced)
    :QDialog(parent) {

    int r=0, i;
    SETTINGS;

    // *******************************************************************************************
    // build the advanced tab
    if (showAdvanced) {
        advancedtablayout = new QGridLayout();
        advancedtabwidget = new QWidget();
        advancedtabwidget->setLayout(advancedtablayout);
        r=0;
        //
        passwordlabel = new QLabel(tr("Advanced Preferences Password:"),this);
        passwordinput = new QLineEdit(QString::null,this);
        passwordinput->setText(settings.value(SETTINGSPREFPASSWORD, "").toString());
        passwordinput->setMaxLength(32);
        passwordinput->setEchoMode(QLineEdit::Password);
        advancedtablayout->addWidget(passwordlabel,r,1,1,1);
        advancedtablayout->addWidget(passwordinput,r,2,1,2);
        //
        r++;
        systemcheckbox = new QCheckBox(tr("Allow SYSTEM statement"),this);
        systemcheckbox->setChecked(settings.value(SETTINGSALLOWSYSTEM, SETTINGSALLOWSYSTEMDEFAULT).toBool());
        advancedtablayout->addWidget(systemcheckbox,r,2,1,2);
        //
        r++;
        settingcheckbox = new QCheckBox(tr("Allow GETSETTING/SETSETTING statements"),this);
        settingcheckbox->setChecked(settings.value(SETTINGSALLOWSETTING, SETTINGSALLOWSETTINGDEFAULT).toBool());
        advancedtablayout->addWidget(settingcheckbox,r,2,1,2);
        //
#ifdef WIN32
#ifndef WIN32PORTABLE
        r++;
        portcheckbox = new QCheckBox(tr("Allow PORTIN/PORTOUT statements"),this);
        portcheckbox->setChecked(settings.value(SETTINGSALLOWPORT, SETTINGSALLOWPORTDEFAULT).toBool());
        advancedtablayout->addWidget(portcheckbox,r,2,1,2);
#endif
#endif
        //
    }


    // *******************************************************************************************
    // build the user tab
    usertablayout = new QGridLayout();
    usertabwidget = new QWidget();
    usertabwidget->setLayout(usertablayout);
    r=0;
    // Startup - restore last windows position on start or check for update
    r++;
    startuplabel = new QLabel(tr("Startup"),this);
    usertablayout->addWidget(startuplabel,r,1,1,1);
    windowsrestorecheckbox = new QCheckBox(tr("Restore last windows position on start"),this);
    windowsrestorecheckbox->setChecked(settings.value(SETTINGSWINDOWSRESTORE, SETTINGSWINDOWSRESTOREDEFAULT).toBool());
    usertablayout->addWidget(windowsrestorecheckbox,r,2,1,1);
    r++;
    checkupdatecheckbox = new QCheckBox(tr("Check for an update on start"),this);
    checkupdatecheckbox->setChecked(settings.value(SETTINGSCHECKFORUPDATE, SETTINGSCHECKFORUPDATEDEFAULT).toBool());
    usertablayout->addWidget(checkupdatecheckbox,r,2,1,2);
    //
    r++;
    {
        int settypeconv;
        typeconvlabel = new QLabel(tr("Runtime handling of bad type conversions:"));
        usertablayout->addWidget(typeconvlabel,r,1,1,1);
        typeconvcombo = new QComboBox();
        typeconvcombo->addItem(tr("Ignore"), SETTINGSERRORNONE);
        typeconvcombo->addItem(tr("Warn"), SETTINGSERRORWARN);
        typeconvcombo->addItem(tr("Error"), SETTINGSERROR);
        // set setting and select
        settypeconv = settings.value(SETTINGSTYPECONV, SETTINGSTYPECONVDEFAULT).toInt();
        int index = typeconvcombo->findData(settypeconv);
        if ( index != -1 ) { // -1 for not found
            typeconvcombo->setCurrentIndex(index);
        }
        // add to layout
        usertablayout->addWidget(typeconvcombo,r,2,1,2);
    }
    //
    r++;
    {
        int setvarnotassigned;
        varnotassignedlabel = new QLabel(tr("Runtime handling of unassigned variables:"));
        usertablayout->addWidget(varnotassignedlabel,r,1,1,1);
        varnotassignedcombo = new QComboBox();
        varnotassignedcombo->addItem(tr("Ignore"), SETTINGSERRORNONE);
        varnotassignedcombo->addItem(tr("Warn"), SETTINGSERRORWARN);
        varnotassignedcombo->addItem(tr("Error"), SETTINGSERROR);
        // set setting and select
        setvarnotassigned = settings.value(SETTINGSVARNOTASSIGNED, SETTINGSVARNOTASSIGNEDDEFAULT).toInt();
        int index = varnotassignedcombo->findData(setvarnotassigned);
        if ( index != -1 ) { // -1 for not found
            varnotassignedcombo->setCurrentIndex(index);
        }
        // add to layout
        usertablayout->addWidget(varnotassignedcombo,r,2,1,2);
    }
    //
    r++;
    saveonruncheckbox = new QCheckBox(tr("Automatically save program when it is successfully run"),this);
    saveonruncheckbox->setChecked(settings.value(SETTINGSIDESAVEONRUN, SETTINGSIDESAVEONRUNDEFAULT).toBool());
    usertablayout->addWidget(saveonruncheckbox,r,2,1,2);
	//
	// decimal digits slider
	r++;
    decdigslabel = new QLabel(tr("Number of digits to print numbers:"),this);
    decdigsvalue = new QLabel("",this);
    QHBoxLayout * decdigslabellayout = new QHBoxLayout();
    decdigslabellayout->addWidget(decdigslabel);
    decdigslabellayout->addWidget(decdigsvalue);
    decdigslabellayout->addStretch();

    decdigsslider = new QSlider(Qt::Horizontal);
	decdigsslider->setMinimum(SETTINGSDECDIGSMIN);
	decdigsslider->setMaximum(SETTINGSDECDIGSMAX);
    decdigsslider->setSingleStep(1);
    decdigsslider->setPageStep(1);
    QLabel *decdigsbefore = new QLabel(QString::number(SETTINGSDECDIGSMIN),this);
    QLabel *decdigsafter = new QLabel(QString::number(SETTINGSDECDIGSMAX),this);
    connect(decdigsslider, SIGNAL(valueChanged(int)), this, SLOT(setDigitsValue(int)));
	QHBoxLayout * decdigssliderlayout = new QHBoxLayout();
	decdigssliderlayout->addWidget(decdigsbefore);
	decdigssliderlayout->addWidget(decdigsslider);
	decdigssliderlayout->addWidget(decdigsafter);
    usertablayout->addLayout(decdigslabellayout,r,1,1,1);
	usertablayout->addLayout(decdigssliderlayout,r,2,1,2);
    i = settings.value(SETTINGSDECDIGS, SETTINGSDECDIGSDEFAULT).toInt();
    decdigsslider->setValue(i);
    setDigitsValue(i);
    decdigsvalue->setText(decdigsvalue->text() + "        "); //ensure space for label to grow without resizing window
    // show trailing .0 on floatingpoint numbers (python style numbers)
    r++;
    floatlocalecheckbox = new QCheckBox(tr("Use localized decimal point on floating point numbers"),this);
    floatlocalecheckbox->setChecked(settings.value(SETTINGSFLOATLOCALE, SETTINGSFLOATLOCALEDEFAULT).toBool());
    usertablayout->addWidget(floatlocalecheckbox,r,2,1,2);
    r++;
    floattailcheckbox = new QCheckBox(tr("Always show decimal point on floating point numbers"),this);
    floattailcheckbox->setChecked(settings.value(SETTINGSFLOATTAIL, SETTINGSFLOATTAILDEFAULT).toBool());
    usertablayout->addWidget(floattailcheckbox,r,2,1,2);

	//
	// speed of next statement in run to breakpoint
	r++;
    debugspeedlabel = new QLabel(tr("Debugging Speed:"),this);
    debugspeedvalue = new QLabel("",this);
    QHBoxLayout * debugspeedlabellayout = new QHBoxLayout();
    debugspeedlabellayout->addWidget(debugspeedlabel);
    debugspeedlabellayout->addWidget(debugspeedvalue);
    debugspeedlabellayout->addStretch();

    debugspeedslider = new QSlider(Qt::Horizontal);
	debugspeedslider->setMinimum(SETTINGSDEBUGSPEEDMIN);
	debugspeedslider->setMaximum(SETTINGSDEBUGSPEEDMAX);
	QLabel *debugspeedbefore = new QLabel(tr("Fast"),this);
	QLabel *debugspeedafter = new QLabel(tr("Slow"),this);
    connect(debugspeedslider, SIGNAL(valueChanged(int)), this, SLOT(setDebugSpeedValue(int)));
	QHBoxLayout * debugspeedsliderlayout = new QHBoxLayout();
	debugspeedsliderlayout->addWidget(debugspeedbefore);
	debugspeedsliderlayout->addWidget(debugspeedslider);
	debugspeedsliderlayout->addWidget(debugspeedafter);
    usertablayout->addLayout(debugspeedlabellayout,r,1,1,1);
	usertablayout->addLayout(debugspeedsliderlayout,r,2,1,2);
    i = settings.value(SETTINGSDEBUGSPEED, SETTINGSDEBUGSPEEDDEFAULT).toInt();
    debugspeedslider->setValue(i);
    setDebugSpeedValue(i);
    debugspeedvalue->setText(debugspeedvalue->text() + "        "); //ensure space for label to grow without resizing window

    //
    // *******************************************************************************************
    // build the sound tab
    soundtablayout = new QGridLayout();
    soundtabwidget = new QWidget();
    soundtabwidget->setLayout(soundtablayout);
    r=0;




    // sound volume slider
    r++;
    soundvolumelabel = new QLabel(tr("Volume:"),this);
    soundvolumevalue = new QLabel("",this);
    QHBoxLayout * soundvolumelabellayout = new QHBoxLayout();
    soundvolumelabellayout->addWidget(soundvolumelabel);
    soundvolumelabellayout->addWidget(soundvolumevalue);
    soundvolumelabellayout->addStretch();
    soundvolumeslider = new QSlider(Qt::Horizontal);
    soundvolumeslider->setMinimum(SETTINGSSOUNDVOLUMEMIN);
    soundvolumeslider->setMaximum(SETTINGSSOUNDVOLUMEMAX);
    soundvolumeslider->setSingleStep(1);
    soundvolumeslider->setPageStep(1);
    QLabel *soundvolumebefore = new QLabel(QString::number(SETTINGSSOUNDVOLUMEMIN),this);
    QLabel *soundvolumeafter = new QLabel(QString::number(SETTINGSSOUNDVOLUMEMAX),this);
    connect(soundvolumeslider, SIGNAL(valueChanged(int)), this, SLOT(setVolumeValue(int)));
    QHBoxLayout * soundvolumesliderlayout = new QHBoxLayout();
    soundvolumesliderlayout->addWidget(soundvolumebefore);
    soundvolumesliderlayout->addWidget(soundvolumeslider);
    soundvolumesliderlayout->addWidget(soundvolumeafter);
    soundtablayout->addLayout(soundvolumelabellayout,r,1,1,1);
    soundtablayout->addLayout(soundvolumesliderlayout,r,2,1,2);
    i = settings.value(SETTINGSSOUNDVOLUME, SETTINGSSOUNDVOLUMEDEFAULT).toInt();
    soundvolumeslider->setValue(i);
    setVolumeValue(i);
    soundvolumevalue->setText(soundvolumevalue->text() + "        "); //ensure space for label to grow without resizing window



    // sample rate
    r++;
    {
        int setsoundsamplerate;
        soundsampleratelabel = new QLabel(tr("Sound sample rate:"));
        soundtablayout->addWidget(soundsampleratelabel,r,1,1,1);
        soundsampleratecombo = new QComboBox();
        soundsampleratecombo->addItem(tr("44100 Hz"), 44100);
        soundsampleratecombo->addItem(tr("32000 Hz"), 32000);
        soundsampleratecombo->addItem(tr("22050 Hz"), 22050);
        soundsampleratecombo->addItem(tr("16000 Hz"), 16000);
        soundsampleratecombo->addItem(tr("11025 Hz"), 11025);
        soundsampleratecombo->addItem(tr("8000 Hz"), 8000);
        soundsampleratecombo->addItem(tr("4400 Hz"), 4400);
        // set setting and select
        setsoundsamplerate = settings.value(SETTINGSSOUNDSAMPLERATE, SETTINGSSOUNDSAMPLERATEDEFAULT).toInt();
        int index = soundsampleratecombo->findData(setsoundsamplerate);
        if ( index != -1 ) { // -1 for not found
            soundsampleratecombo->setCurrentIndex(index);
        }
        // add to layout
        soundtablayout->addWidget(soundsampleratecombo,r,2,1,2);
    }



    // sound normalize slider
    r++;
    soundnormalizelabel = new QLabel(tr("Min. period to normalize louder sounds:"),this);
    soundnormalizevalue = new QLabel("",this);
    QHBoxLayout * soundnormalizelabellayout = new QHBoxLayout();
    soundnormalizelabellayout->addWidget(soundnormalizelabel);
    soundnormalizelabellayout->addWidget(soundnormalizevalue);
    soundnormalizelabellayout->addStretch();
    soundnormalizeslider = new QSlider(Qt::Horizontal);
    soundnormalizeslider->setMinimum(SETTINGSSOUNDNORMALIZEMIN/10);
    soundnormalizeslider->setMaximum(SETTINGSSOUNDNORMALIZEMAX/10);
    soundnormalizeslider->setSingleStep(1);
    soundnormalizeslider->setPageStep(10);
    QLabel *soundnormalizebefore = new QLabel(QString::number(SETTINGSSOUNDNORMALIZEMIN),this);
    QLabel *soundnormalizeafter = new QLabel(QString::number(SETTINGSSOUNDNORMALIZEMAX),this);
    connect(soundnormalizeslider, SIGNAL(valueChanged(int)), this, SLOT(setNormalizeValue(int)));
    QHBoxLayout * soundnormalizesliderlayout = new QHBoxLayout();
    soundnormalizesliderlayout->addWidget(soundnormalizebefore);
    soundnormalizesliderlayout->addWidget(soundnormalizeslider);
    soundnormalizesliderlayout->addWidget(soundnormalizeafter);
    soundtablayout->addLayout(soundnormalizelabellayout,r,1,1,1);
    soundtablayout->addLayout(soundnormalizesliderlayout,r,2,1,2);
    i = settings.value(SETTINGSSOUNDNORMALIZE, SETTINGSSOUNDNORMALIZEDEFAULT).toInt()/10;
    soundnormalizeslider->setValue(i);
    setNormalizeValue(i);
    soundnormalizevalue->setText(soundnormalizevalue->text() + "        "); //ensure space for label to grow without resizing window






    // sound volume restore slider
    r++;
    soundvolumerestorelabel = new QLabel(tr("Period to restore volume after louder sound:"),this);
    soundvolumerestorevalue = new QLabel("",this);
    QHBoxLayout * soundvolumerestorelabellayout = new QHBoxLayout();
    soundvolumerestorelabellayout->addWidget(soundvolumerestorelabel);
    soundvolumerestorelabellayout->addWidget(soundvolumerestorevalue);
    soundvolumerestorelabellayout->addStretch();
    soundvolumerestoreslider = new QSlider(Qt::Horizontal);
    soundvolumerestoreslider->setMinimum(SETTINGSSOUNDVOLUMERESTOREMIN/10);
    soundvolumerestoreslider->setMaximum(SETTINGSSOUNDVOLUMERESTOREMAX/10);
    soundvolumerestoreslider->setSingleStep(1);
    soundvolumerestoreslider->setPageStep(10);
    QLabel *soundvolumerestorebefore = new QLabel(QString::number(SETTINGSSOUNDVOLUMERESTOREMIN),this);
    QLabel *soundvolumerestoreafter = new QLabel(QString::number(SETTINGSSOUNDVOLUMERESTOREMAX),this);
    connect(soundvolumerestoreslider, SIGNAL(valueChanged(int)), this, SLOT(setVolumeRestoreValue(int)));
    QHBoxLayout * soundvolumerestoresliderlayout = new QHBoxLayout();
    soundvolumerestoresliderlayout->addWidget(soundvolumerestorebefore);
    soundvolumerestoresliderlayout->addWidget(soundvolumerestoreslider);
    soundvolumerestoresliderlayout->addWidget(soundvolumerestoreafter);
    soundtablayout->addLayout(soundvolumerestorelabellayout,r,1,1,1);
    soundtablayout->addLayout(soundvolumerestoresliderlayout,r,2,1,2);
    i = settings.value(SETTINGSSOUNDVOLUMERESTORE, SETTINGSSOUNDVOLUMERESTOREDEFAULT).toInt()/10;
    soundvolumerestoreslider->setValue(i);
    setVolumeRestoreValue(i);
    soundvolumerestorevalue->setText(soundvolumerestorevalue->text() + "        "); //ensure space for label to grow without resizing window










#ifdef ESPEAK
    r++;
    {
#ifdef WIN32
        // use program install folder
        espeak_Initialize(AUDIO_OUTPUT_SYNCH_PLAYBACK,0,(char *) QFileInfo(QCoreApplication::applicationFilePath()).absolutePath().toUtf8().data(),0);
#else
        // use default path for espeak-data
        espeak_Initialize(AUDIO_OUTPUT_SYNCH_PLAYBACK,0,NULL,0);
#endif

        QString setvoice;
        voicelabel = new QLabel(tr("SAY Voice:"));
        soundtablayout->addWidget(voicelabel,r,1,1,1);
        voicecombo = new QComboBox();

        const espeak_VOICE **voices;
        voices = espeak_ListVoices(NULL);

        voicecombo->addItem(QString("default"),	QString("default"));
        for(int i=0; voices[i] != NULL; i++) {
            QString name = voices[i]->name;
            QString lang = (voices[i]->languages + 1);
            voicecombo->addItem( lang + " (" + name + ")",	name);
        }

        // set setting and select
        setvoice = settings.value(SETTINGSESPEAKVOICE, SETTINGSESPEAKVOICEDEFAULT).toString();
        int index = voicecombo->findData(setvoice);
        if ( index != -1 ) { // -1 for not found
            voicecombo->setCurrentIndex(index);
        }
        // add to layout
        soundtablayout->addWidget(voicecombo,r,2,1,2);
    }
#endif

    // *******************************************************************************************
    // build the printer tab
    printertablayout = new QGridLayout();
    printertabwidget = new QWidget();
    printertabwidget->setLayout(printertablayout);
    r=0;
    {
        int dflt = -1;
        int setprinter;
        printerslabel = new QLabel(tr("Printer:"));
        printertablayout->addWidget(printerslabel,r,1,1,1);
        printerscombo = new QComboBox();
        // build list of printers
        QList<QPrinterInfo> printerList=QPrinterInfo::availablePrinters();
        for (int i=0; i< printerList.count(); i++) {
            printerscombo->addItem(printerList[i].printerName(),i);
            if (printerList[i].isDefault()) dflt = i;
        }
        printerscombo->addItem(tr("PDF File Output"),-1);
        // set setting and select
        setprinter = settings.value(SETTINGSPRINTERPRINTER, dflt).toInt();
        if (setprinter<-1||setprinter>=printerList.count()) setprinter = dflt;
        int index = printerscombo->findData(setprinter);
        if ( index != -1 ) { // -1 for not found
            printerscombo->setCurrentIndex(index);
        }
        // add to layout
        printertablayout->addWidget(printerscombo,r,2,1,2);
    }
    //
    r++;
    {
        int setpaper;
        paperlabel = new QLabel(tr("Paper:"));
        printertablayout->addWidget(paperlabel,r,1,1,1);
        papercombo = new QComboBox();
        papercombo->addItem("A0 (841 x 1189 mm)",	QPrinter::A0);
        papercombo->addItem("A1 (594 x 841 mm)", QPrinter::A1);
        papercombo->addItem("A2 (420 x 594 mm", QPrinter::A2);
        papercombo->addItem("A3 (297 x 420 mm", QPrinter::A3);
        papercombo->addItem("A4 (210 x 297 mm, 8.26 x 11.69 inches", QPrinter::A4);
        papercombo->addItem("A5 (148 x 210 mm", QPrinter::A5);
        papercombo->addItem("A6 (105 x 148 mm", QPrinter::A6);
        papercombo->addItem("A7 (74 x 105 mm", QPrinter::A7);
        papercombo->addItem("A9 (52 x 74 mm", QPrinter::A8);
        papercombo->addItem("A9 (37 x 52 mm", QPrinter::A9);
        papercombo->addItem("B0 (1000 x 1414 mm", QPrinter::B0);
        papercombo->addItem("B1 (707 x 1000 mm", QPrinter::B1);
        papercombo->addItem("B2 (500 x 707 mm", QPrinter::B2);
        papercombo->addItem("B3 (353 x 500 mm", QPrinter::B3);
        papercombo->addItem("B4 (250 x 353 mm", QPrinter::B4);
        papercombo->addItem("B5 (176 x 250 mm, 6.93 x 9.84 inches", QPrinter::B5);
        papercombo->addItem("B6 (125 x 176 mm", QPrinter::B6);
        papercombo->addItem("B7 (88 x 125 mm", QPrinter::B7);
        papercombo->addItem("B8 (62 x 88 mm", QPrinter::B8);
        papercombo->addItem("B9 (33 x 62 mm", QPrinter::B9);
        papercombo->addItem("B10 (31 x 44 mm", QPrinter::B10);
        papercombo->addItem("#5 Envelope (163 x 229 mm", QPrinter::C5E);
        papercombo->addItem("#10 Envelope (105 x 241 mm", QPrinter::Comm10E);
        papercombo->addItem("DLE (110 x 220 mm", QPrinter::DLE);
        papercombo->addItem("Executive (7.5 x 10 inches, 190.5 x 254 mm", QPrinter::Executive);
        papercombo->addItem("Folio (210 x 330 mm", QPrinter::Folio);
        papercombo->addItem("Ledger (431.8 x 279.4 mm", QPrinter::Ledger);
        papercombo->addItem("Legal (8.5 x 14 inches, 215.9 x 355.6 mm", QPrinter::Legal);
        papercombo->addItem("Letter (8.5 x 11 inches, 215.9 x 279.4 mm", QPrinter::Letter);
        papercombo->addItem("Tabloid (279.4 x 431.8 mm", QPrinter::Tabloid);
        // set setting and select
        setpaper = settings.value(SETTINGSPRINTERPAPER, SETTINGSPRINTERPAPERDEFAULT).toInt();
        int index = papercombo->findData(setpaper);
        if ( index != -1 ) { // -1 for not found
            papercombo->setCurrentIndex(index);
        }
        // add to layout
        printertablayout->addWidget(papercombo,r,2,1,2);
    }
    //
    r++;
    {
        pdffilelabel = new QLabel(tr("PDF File Name:"),this);
        pdffileinput = new QLineEdit(QString::null,this);
        pdffileinput->setText(settings.value(SETTINGSPRINTERPDFFILE, "").toString());
        pdffileinput->setMaxLength(96);
        printertablayout->addWidget(pdffilelabel,r,1,1,1);
        printertablayout->addWidget(pdffileinput,r,2,1,2);
    }
    //
    r++;
    {
        resolutiongroup = new QGroupBox(tr("Printer Resolution:"));
        resolutionhigh = new QRadioButton(tr("High"));
        resolutionscreen = new QRadioButton(tr("Screen"));
        QVBoxLayout *resolutionbox = new QVBoxLayout;
        resolutionbox->addWidget(resolutionhigh);
        resolutionbox->addWidget(resolutionscreen);
        resolutiongroup->setLayout(resolutionbox);
        printertablayout->addWidget(resolutiongroup,r,2,1,2);
        resolutionhigh->setChecked(settings.value(SETTINGSPRINTERRESOLUTION, SETTINGSPRINTERRESOLUTIONDEFAULT)==QPrinter::HighResolution);
        resolutionscreen->setChecked(settings.value(SETTINGSPRINTERRESOLUTION, SETTINGSPRINTERRESOLUTIONDEFAULT)==QPrinter::ScreenResolution);
    }
    //
    r++;
    {
        orientgroup = new QGroupBox(tr("Orientation:"));
        orientportrait = new QRadioButton(tr("Portrait"));
        orientlandscape = new QRadioButton(tr("Landscape"));
        QVBoxLayout *orientbox = new QVBoxLayout;
        orientbox->addWidget(orientportrait);
        orientbox->addWidget(orientlandscape);
        orientgroup->setLayout(orientbox);
        printertablayout->addWidget(orientgroup,r,2,1,2);
        orientportrait->setChecked(settings.value(SETTINGSPRINTERORIENT, SETTINGSPRINTERORIENTDEFAULT)==QPrinter::Portrait);
        orientlandscape->setChecked(settings.value(SETTINGSPRINTERORIENT, SETTINGSPRINTERORIENTDEFAULT)==QPrinter::Landscape);
    }


    // *******************************************************************************************
    // *******************************************************************************************
    // *******************************************************************************************
    // now that we have the tab layouts built - make the main window
    QTabWidget * tabs = new QTabWidget();
    tabs->addTab(usertabwidget,tr("User"));
    tabs->addTab(soundtabwidget,tr("Sound"));
    tabs->addTab(printertabwidget,tr("Printing"));
    if (showAdvanced) tabs->addTab(advancedtabwidget,tr("Advanced"));

    QGridLayout * mainlayout = new QGridLayout();
    mainlayout->addWidget(tabs,0,1,1,3);

    cancelbutton = new QPushButton(tr("Cancel"), this);
    connect(cancelbutton, SIGNAL(clicked()), this, SLOT (clickCancelButton()));
    savebutton = new QPushButton(tr("Save"), this);
    connect(savebutton, SIGNAL(clicked()), this, SLOT (clickSaveButton()));
    mainlayout->addWidget(cancelbutton,1,2,1,1);
    mainlayout->addWidget(savebutton,1,3,1,1);
    savebutton->setDefault(true);

    this->setLayout(mainlayout);

    move(settings.value(SETTINGSPREFPOS, QPoint(200, 200)).toPoint());
    setWindowTitle(tr("BASIC-256 Preferences and Settings"));
    this->show();

}

void PreferencesWin::clickCancelButton() {
    close();
}

void PreferencesWin::clickSaveButton() {
    bool validate = true;

    SETTINGS;

    // validate file name if PDF
    if (printerscombo->itemData(printerscombo->currentIndex())==-1 && pdffileinput->text()=="") {
        //
        QMessageBox::information(this, tr(SETTINGSAPP), tr("File name required for PDF output."),QMessageBox::Ok, QMessageBox::Ok);
        validate = false;
    }
    // add other validation here

    if(validate) {

        // *******************************************************************************************
        // advanced settings
        if(advancedtabwidget) {
            // only changed advanced settings if they are  shown
            //
            // if password has changed - digest and save
            QString currentpw = settings.value(SETTINGSPREFPASSWORD, "").toString();
            if(QString::compare(currentpw,passwordinput->text())!=0) {
                QString pw("");
                if(passwordinput->text().length()!=0) {
                    pw = MD5(passwordinput->text().toUtf8().data()).hexdigest();
                }
                settings.setValue(SETTINGSPREFPASSWORD, pw);
            }
            //
            settings.setValue(SETTINGSALLOWSYSTEM, systemcheckbox->isChecked());
            settings.setValue(SETTINGSALLOWSETTING, settingcheckbox->isChecked());
#ifdef WIN32
#ifndef WIN32PORTABLE
            settings.setValue(SETTINGSALLOWPORT, portcheckbox->isChecked());
#endif
#endif
        }


        // *******************************************************************************************
        // user settings
        settings.setValue(SETTINGSIDESAVEONRUN, saveonruncheckbox->isChecked());
        if (typeconvcombo->currentIndex()!=-1) {
            settings.setValue(SETTINGSTYPECONV, typeconvcombo->itemData(typeconvcombo->currentIndex()));
        }
        if (varnotassignedcombo->currentIndex()!=-1) {
            settings.setValue(SETTINGSVARNOTASSIGNED, varnotassignedcombo->itemData(varnotassignedcombo->currentIndex()));
        }
        settings.setValue(SETTINGSDECDIGS, decdigsslider->value());
        settings.setValue(SETTINGSDEBUGSPEED, debugspeedslider->value());
        settings.setValue(SETTINGSFLOATTAIL, floattailcheckbox->isChecked());
        settings.setValue(SETTINGSFLOATLOCALE, floatlocalecheckbox->isChecked());
        settings.setValue(SETTINGSWINDOWSRESTORE, windowsrestorecheckbox->isChecked());
        settings.setValue(SETTINGSCHECKFORUPDATE, checkupdatecheckbox->isChecked());

        // *******************************************************************************************
        // sound settings
        settings.setValue(SETTINGSSOUNDVOLUME, soundvolumeslider->value());
        settings.setValue(SETTINGSSOUNDNORMALIZE, soundnormalizeslider->value()*10);
        settings.setValue(SETTINGSSOUNDVOLUMERESTORE, soundvolumerestoreslider->value()*10);
        if (soundsampleratecombo->currentIndex()!=-1) {
            settings.setValue(SETTINGSSOUNDSAMPLERATE, soundsampleratecombo->itemData(soundsampleratecombo->currentIndex()));
        }
#ifdef ESPEAK
        //
        if (voicecombo->currentIndex()!=-1) {
            settings.setValue(SETTINGSESPEAKVOICE, voicecombo->itemData(voicecombo->currentIndex()));
        }
#endif

        // *******************************************************************************************
        // printer settings
        if (printerscombo->currentIndex()!=-1) {
            settings.setValue(SETTINGSPRINTERPRINTER, printerscombo->itemData(printerscombo->currentIndex()));
        }
        //
        if (papercombo->currentIndex()!=-1) {
            settings.setValue(SETTINGSPRINTERPAPER, papercombo->itemData(papercombo->currentIndex()));
        }
        //
        settings.setValue(SETTINGSPRINTERPDFFILE, pdffileinput->text());
        //
        if (resolutionhigh->isChecked()) settings.setValue(SETTINGSPRINTERRESOLUTION, QPrinter::HighResolution);
        if (resolutionscreen->isChecked()) settings.setValue(SETTINGSPRINTERRESOLUTION, QPrinter::ScreenResolution);
        //
        if (orientportrait->isChecked()) settings.setValue(SETTINGSPRINTERORIENT, QPrinter::Portrait);
        if (orientlandscape->isChecked()) settings.setValue(SETTINGSPRINTERORIENT, QPrinter::Landscape);


        // *******************************************************************************************
        // all done
        QMessageBox::information(this, tr(SETTINGSAPP), tr("Preferences and settings have been saved."),QMessageBox::Ok, QMessageBox::Ok);
        //
        close();
    }
}

void PreferencesWin::closeEvent(QCloseEvent *e) {
    (void) e;
    // save current screen posision
    SETTINGS;
    //settings.setValue(SETTINGSPREFSIZE, size());
    settings.setValue(SETTINGSPREFPOS, pos());

}

void PreferencesWin::setDigitsValue(int i){
    QString t = QString::number(i) + " " + tr("digits");
    QToolTip::showText(QCursor::pos(), t);
    decdigsvalue->setText(t);
    decdigsslider->setToolTip(t);
}

void PreferencesWin::setDebugSpeedValue(int i){
    QString t = tr("pause") + " " + QString::number(i) + " " + tr("ms");
    QToolTip::showText(QCursor::pos(),t);
    debugspeedvalue->setText(t);
    debugspeedslider->setToolTip(t);
}

void PreferencesWin::setVolumeValue(int i){
    QString t = QString::number(i);
    QToolTip::showText(QCursor::pos(), t);
    soundvolumevalue->setText(t);
    soundvolumeslider->setToolTip(t);
}

void PreferencesWin::setNormalizeValue(int i){
    QString t = QString::number(i*10) + " " + tr("ms");
    QToolTip::showText(QCursor::pos(), t);
    soundnormalizevalue->setText(t);
    soundnormalizeslider->setToolTip(t);
}

void PreferencesWin::setVolumeRestoreValue(int i){
    QString t = QString::number(i*10) + " " + tr("ms");
    QToolTip::showText(QCursor::pos(), t);
    soundvolumerestorevalue->setText(t);
    soundvolumerestoreslider->setToolTip(t);
}

