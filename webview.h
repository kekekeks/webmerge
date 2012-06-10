/*
webview.h
Author:
     Nikita Tsukanov <keks9n@gmail.com>

Copyright (c) 2011 Nikita Tsukanov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWidget>
#include <QWebView>

class WebView : public QWebView
{
	Q_OBJECT
private slots:
	void onLoad(bool ok);
	virtual void contextMenuEvent(QContextMenuEvent *ev);
public:
	WebView(QWidget *parent = 0);
	~WebView();
};

#endif // WEBVIEW_H
