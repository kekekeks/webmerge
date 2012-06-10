/*
webview.cpp
Author:
     Nikita Tsukanov <keks9n@gmail.com>

Copyright (c) 2012 Nikita Tsukanov

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

#include "webview.h"
#define protected public
#include <QThread>
#undef protected
#include <QWebFrame>
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include <QWebElement>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>
#include <QMap>
#include <stdio.h>
void QWEBKIT_EXPORT qt_webpage_setGroupName(QWebPage* page, const QString& groupName);
WebView::WebView(QWidget *parent):QWebView(parent)
{
	settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
	page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	qt_webpage_setGroupName(page(), "Muahahaha");
	connect(this, SIGNAL(loadFinished(bool)), SLOT(onLoad(bool)));

}

WebView::~WebView()
{
	
}

QMap<QString, QByteArray> cache;
bool downloadFile(QString url, QByteArray *ret)
{
	if(cache.keys().contains(url))
	{
		*ret=cache.value(url);
		return true;
	}
	printf("Downloading file %s\n", url.toLocal8Bit().data());
	fflush(stdout);
	QNetworkAccessManager manager;
	int tries=0;
	for(;;)
	{
		QNetworkRequest request=QNetworkRequest((QUrl(url)));
		QNetworkReply*reply=manager.get(request);
		while(!reply->isFinished())
		{
			QThread::sleep(0);
			QApplication::processEvents();
		}
		QString err=reply->errorString();
		if(reply->error()!=QNetworkReply::NoError)
		{
			delete reply;
			tries++;
			if(tries>=3)
			{
					return false;
			}
			continue;
		}
		*ret=reply->readAll();
		cache.insert(url, *ret);
		delete reply;
		return true;
	}
}


bool downloadString(QString url, QString *ret)
{
	QByteArray data;
	if(!downloadFile(url, &data))
		return false;
	*ret=QString::fromUtf8(data.data(), data.length());
	return true;
}

bool downloadImage(QString url, QString*ret)
{
	QString prefix="data:image/png;base64,";
	QByteArray data;
	if(!downloadFile(url, &data))
		return false;
	*ret = prefix + data.toBase64();
	return true;
}

QString urlCombine(QString base, QString path)
{
	if(path.length()==0)
		return "";
	auto baseUrl = QUrl(base);
	if(path.at(0)=='/')
	{
		int port=baseUrl.port();
		if(port==-1)
			port=80;
		return baseUrl.scheme() + "://" + baseUrl.host() + ":" + QString::number(port) + path;
	}
	auto b = base.left(base.lastIndexOf('/')+1);
	return b + path;
}

QString transformCSS(QString css, QString baseUrl)
{
	QStringList source=css.split('\n', QString::SkipEmptyParts);
	QStringList dest;
	foreach(QString s, source)
	{
		if(s.contains("url("))
		{
			//url\(["']*([^"')])*["']*\)
			QString xexp="url\\([\"']*([^\"')]*)[\"']*\\)";
			QRegExp exp(xexp);
			if(exp.indexIn(s)!=-1)
			{
				QString url=exp.cap(1);
				url=urlCombine(baseUrl, url);
				QString imgData;
				if(!downloadImage(url, &imgData))
				{
					printf("Unable to download %s\n", url.toLocal8Bit().data());
				}
				else
				{
					s=s.replace(exp, "url('"+imgData+"')");
				}
			}

		}
		dest.append(s);
	}
	return dest.join("\n");

}

void WebView::onLoad(bool ok)
{
	printf("Page loaded, status: %i\n", (int)ok);
	auto frame=page()->currentFrame();
	auto durl=page()->currentFrame()->url().toString();
	foreach (QWebElement el, frame->findAllElements("script"))
	{
		auto src = el.attribute("src", "");
		if(src!="")
		{
			QString scriptData;
			src=urlCombine(durl, src);
			if(!downloadString(src, &scriptData))
			{
				printf("Unable to download %s\n", src.toLocal8Bit().data());
			}
			else
			{
				el.removeAttribute("src");
				el.setInnerXml(scriptData);
			}
		}
	}
	foreach(QWebElement el, frame->findAllElements("link"))
	{
		if(el.attribute("rel", "")!="stylesheet")
			continue;
		QString styleData;
		QString src=el.attribute("href", "");
		src=urlCombine(durl, src);
		if(!downloadString(src, &styleData))
		{
			printf("Unable to download %s\n", src.toLocal8Bit().data());
		}
		else
		{
			el.setOuterXml("<style>"+transformCSS(styleData, src)+"</style>");
		}
	}
	foreach(QWebElement el, frame->findAllElements("img"))
	{
		auto src=el.attribute("src", "");
		QString imgData;
		src=urlCombine(durl, src);
		if(!downloadImage(src, &imgData))
		{
			printf("Unable to download %s\n", src.toLocal8Bit().data());
		}
		else
		{
			el.setAttribute("src", imgData);
		}

	}
	auto path = QApplication::arguments().at(2);
	QFile file(path);
	if(!file.open(QFile::WriteOnly|QFile::Truncate))
	{
		printf("Unable to open file %s\n", path.toLocal8Bit().data());
		exit(1);
	}
	file.write(frame->toHtml().toUtf8());
	file.close();

	settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
	exit (0);
}

void WebView::contextMenuEvent(QContextMenuEvent *ev)
{
	QMenu *menu=page()->createStandardContextMenu();
	menu->addAction(page()->action(QWebPage::InspectElement));
	menu->exec(ev->globalPos());
	delete menu;
}
