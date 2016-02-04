#!/bin/sh

CP=bin
CP=$CP:jpathwatch-0-94.jar
CP=$CP:jfreechart-1.0.13.jar
CP=$CP:jcommon-1.0.16.jar

java -cp $CP watchPlot ISUS uMol 6 "\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}.\d{3}.*SATNLF0188.*" . ".*log"
