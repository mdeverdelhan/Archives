/*
 * Copyright (C) 2009 Marc de Verdelhan (http://www.verdelhan.eu/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.dailybashquote;

import com.android.dailybashquote.BashHelper.ApiException;
import com.android.dailybashquote.BashHelper.ParseException;

import android.app.Service;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;
import android.widget.RemoteViews;


/**
 * Define a simple widget that shows the Bash.org "Quote of the day." To build
 * an update we spawn a background {@link Service} to perform the API queries.
 */
public class BashQuoteWidget extends AppWidgetProvider {
    @Override
    public void onUpdate(Context context, AppWidgetManager appWidgetManager,
            int[] appWidgetIds) {
        // To prevent any ANR timeouts, we perform the update in a service
        context.startService(new Intent(context, UpdateService.class));
    }
    
    public static class UpdateService extends Service {
        @Override
        public void onStart(Intent intent, int startId) {
            // Build the widget update for today
            RemoteViews updateViews = buildUpdate(this);
            
            // Push update for this widget to the home screen
            ComponentName thisWidget = new ComponentName(this, BashQuoteWidget.class);
            AppWidgetManager manager = AppWidgetManager.getInstance(this);
            manager.updateAppWidget(thisWidget, updateViews);
        }

        /**
         * Build a widget update to show the current Bash.org
         * "Quote of the day." Will block until the online API returns.
         */
        public RemoteViews buildUpdate(Context context) {

            RemoteViews updateViews = null;
            String pageContent = "";
            
            try {
                // Try querying the Bash API for today's quote
                BashHelper.prepareUserAgent(context);
                pageContent = BashHelper.getRandomPageContent();
            } catch (ApiException e) {
                Log.e("BashQuoteWidget", "Couldn't contact API", e);
            } catch (ParseException e) {
                Log.e("BashQuoteWidget", "Couldn't parse API response", e);
            }
            
            String[] quotes = pageContent.split("class=\"quote\"");
            String quote = quotes[1];
            int start = quote.indexOf("class=\"qt\"") + 11;
            int end = quote.indexOf("</p>", start);
            String actualQuote = quote.substring(start, end);
            actualQuote = actualQuote.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&quot;","\"").replaceAll("&amp;","&");
            actualQuote = actualQuote.replaceAll("<br />","");

            //Log.i("BashQuoteWidget - actualQuote", actualQuote);

            if (actualQuote != null && !"".equals(actualQuote)) {
                // Build an update that holds the updated widget contents
                updateViews = new RemoteViews(context.getPackageName(), R.layout.widget_quote);
                updateViews.setTextViewText(R.id.quote, actualQuote.trim());
                
            } else {
                // Didn't find quote of day, so show error message
                updateViews = new RemoteViews(context.getPackageName(), R.layout.widget_message);
                CharSequence errorMessage = context.getText(R.string.widget_error);
                updateViews.setTextViewText(R.id.message, errorMessage);
            }
            return updateViews;
        }
        
        @Override
        public IBinder onBind(Intent intent) {
            // We don't need to bind to this service
            return null;
        }
    }
}
