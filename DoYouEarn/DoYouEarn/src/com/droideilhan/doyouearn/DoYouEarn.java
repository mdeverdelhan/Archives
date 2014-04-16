/*
* Copyright (C) 2010 Marc de Verdelhan (http://www.verdelhan.eu/)
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.droideilhan.doyouearn;

import com.admob.android.ads.AdView;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.webkit.WebView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;

public class DoYouEarn extends Activity implements OnClickListener {
    
    /**
     * Devises
     */
    private static final String DEVISE_USD = "USD ($)";
    private static final String DEVISE_EUR = "EUR (€)";
    
    private EditText editTextSalaire; // EditText du salaire
    private Spinner spinnerDevise; // Spinner de choix de la devise
    private Button buttonCalculer; // Bouton Calculer
    private WebView webViewSalaireChangeant; // WebView du salaire changeant (i.e. WebView qui affiche les mises à jours du salaire gagné (en monnaie))
    
    private Float salaireMensuel; // Salaire mensuel
    private TextView textViewSalaireGagne;
    private Thread threadMajSalaire; // Thread de mise à jour du salaire

    private AdView adStats;
    
    /**
     * Statistiques
     */
    private TextView textViewVotreSalaire;
    private TextView textViewSalaireJour;
    private WebView webViewSalaireJour; // WebView du salaire par jour
    private TextView textViewSalaireHeure;
    private WebView webViewSalaireHeure; // WebView du salaire par heure
    private TextView textViewSalaireMinute;
    private WebView webViewSalaireMinute; // WebView du salaire par minute
    private TextView textViewSalaireSeconde;
    private WebView webViewSalaireSeconde; // WebView du salaire par seconde
    private TextView textViewAvantStats;
    
    /**
     * Aide
     */
    private Button buttonAutresApps;
    
    /**
     * Appelée à la création de l'activité
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        TabHost tabs = (TabHost) findViewById(R.id.tabhost);
        tabs.setup();
        
        // Création du premier onglet
        TabHost.TabSpec tabSpec = tabs.newTabSpec(getString(R.string.tag_onglet1));
        tabSpec.setContent(R.id.tab1);
        tabSpec.setIndicator(getString(R.string.label_onglet1), this.getResources().getDrawable(R.drawable.salaire));
        tabs.addTab(tabSpec);

        // Création du deuxième onglet
        tabSpec = tabs.newTabSpec(getString(R.string.tag_onglet2));
        tabSpec.setContent(R.id.tab2);
        tabSpec.setIndicator(getString(R.string.label_onglet2), this.getResources().getDrawable(R.drawable.statistiques));
        tabs.addTab(tabSpec);

        // Création du troisième onglet
        tabSpec = tabs.newTabSpec(getString(R.string.tag_onglet3));
        tabSpec.setContent(R.id.tab3);
        tabSpec.setIndicator(getString(R.string.label_onglet3), this.getResources().getDrawable(R.drawable.aide));
        tabs.addTab(tabSpec);
        
        tabs.setCurrentTab(0); // Au départ, affichage du premier onglet
        
        // Création de l'EditText du salaire
        editTextSalaire = (EditText) findViewById(R.id.editTextSalaire);
        
        // Création de la liste de choix de la devise
        spinnerDevise = (Spinner) findViewById(R.id.spinnerDevise);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this, R.array.liste_devises, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinnerDevise.setAdapter(adapter);

        // Création du bouton Calculer
        buttonCalculer = (Button) findViewById(R.id.buttonCalculer);
        buttonCalculer.setOnClickListener(this);

        // Création de la WebView du salaire changeant
        textViewSalaireGagne = (TextView) findViewById(R.id.textViewSalaireGagne);
        webViewSalaireChangeant = (WebView) findViewById(R.id.webViewSalaireChangeant);
        webViewSalaireChangeant.setBackgroundColor(0); // WebView transparente
        
        // Création des statistiques   
        adStats = (AdView) findViewById(R.id.adStats); // Création de la pub statistiques
        textViewVotreSalaire = (TextView) findViewById(R.id.textViewVotreSalaire);
        textViewSalaireJour = (TextView) findViewById(R.id.textViewSalaireJour);
        webViewSalaireJour = (WebView) findViewById(R.id.webViewSalaireJour);
        webViewSalaireJour.setBackgroundColor(0);
        textViewSalaireHeure = (TextView) findViewById(R.id.textViewSalaireHeure);
        webViewSalaireHeure = (WebView) findViewById(R.id.webViewSalaireHeure);
        webViewSalaireHeure.setBackgroundColor(0);
        textViewSalaireMinute = (TextView) findViewById(R.id.textViewSalaireMinute);
        webViewSalaireMinute = (WebView) findViewById(R.id.webViewSalaireMinute);
        webViewSalaireMinute.setBackgroundColor(0);
        textViewSalaireSeconde = (TextView) findViewById(R.id.textViewSalaireSeconde);
        webViewSalaireSeconde = (WebView) findViewById(R.id.webViewSalaireSeconde);
        webViewSalaireSeconde.setBackgroundColor(0);
        textViewAvantStats = (TextView) findViewById(R.id.textViewAvantStats);

        // Création de l'aide
        buttonAutresApps = (Button) findViewById(R.id.buttonAutresApps);
        buttonAutresApps.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        if(buttonCalculer.equals(v)) { // Clic sur le bouton Calculer
            
            // Vérification de la valeur entrée pour le salaire mensuel
            try {
                salaireMensuel = Float.parseFloat(editTextSalaire.getText().toString());
                if(salaireMensuel < 1.0) {
                    showMessageDialog(R.string.msg_erreur_salaire_non_numerique);
                    return;
                } else if(salaireMensuel >= 10000000.0) {
                    showMessageDialog(R.string.msg_erreur_salaire_trop_eleve);
                    return;
                }
            } catch(NumberFormatException nfe) {
                showMessageDialog(R.string.msg_erreur_salaire_non_numerique);
                return;
            }
            // Vérifications passées avec succès
            
            // Si le thread existait dejà, on l'interrompt
            if(threadMajSalaire != null && threadMajSalaire.isAlive()) {
                threadMajSalaire.interrupt();
            }
            
            adStats.setVisibility(View.VISIBLE);
            textViewSalaireGagne.setVisibility(View.VISIBLE);
            
            // Création du thread de mise à jour du salaire
            threadMajSalaire = new Thread() {
                
                private boolean continuerAffichage = true;
                /**
                 * Run du thread de mise à jour du salaire
                 */
                public void run() {
                    
                    // Calcul des différents salaires
                    float salaireParJour = DoYouEarn.this.salaireMensuel / 22;
                    float salaireParHeure = salaireParJour / 8;
                    float salaireParMinute = salaireParHeure / 60;
                    float salaireParSeconde = salaireParMinute / 60;
                    float salaireParMilliseconde = salaireParSeconde / 1000;
                    
                    long startTime = System.currentTimeMillis();
                    long currentTime;
                    float montantGagne;
                    
                    // Récupération de la devise choisie
                    String devise = (String) spinnerDevise.getSelectedItem();
                    if(DEVISE_USD.compareTo(devise) == 0) {
                        // On est en dollars US
                        // Chargement des statistiques
                        loadStatistiquesUSD(salaireParJour, salaireParHeure, salaireParMinute, salaireParSeconde);
                        while(continuerAffichage) {
                            // Calcul du montant gagné
                            currentTime = System.currentTimeMillis();
                            montantGagne = (currentTime-startTime) * salaireParMilliseconde;
                                                
                            // Mise à jour de la WebView
                            webViewSalaireChangeant.loadDataWithBaseURL("file:///android_asset/", getHtmlDataFromMontantUSD(montantGagne), "text/html", "UTF-8", "");
                            try {
                                // Mise à jour 5 fois par seconde
                                Thread.sleep(200);
                            } catch (InterruptedException e) {
                                // TODO Auto-generated catch block
                                e.printStackTrace();
                            }
                        }
                    } else if(DEVISE_EUR.compareTo(devise) == 0) {
                        // On est en euros
                        // Chargement des statistiques
                        loadStatistiquesEUR(salaireParJour, salaireParHeure, salaireParMinute, salaireParSeconde);
                        while(continuerAffichage) {
                            // Calcul du montant gagné
                            currentTime = System.currentTimeMillis();
                            montantGagne = (currentTime-startTime) * salaireParMilliseconde;
                                                
                            // Mise à jour de la WebView
                            webViewSalaireChangeant.loadDataWithBaseURL("file:///android_asset/", getHtmlDataFromMontantEUR(montantGagne), "text/html", "UTF-8", "");
                            try {
                                // Mise à jour 5 fois par seconde
                                Thread.sleep(200);
                            } catch (InterruptedException e) {
                                // TODO Auto-generated catch block
                                e.printStackTrace();
                            }
                        }
                    } else {
                        // TODO Autres devises - A traiter
                    }
                    
                    
                }
                
                @Override
                public void interrupt() {
                    super.interrupt();
                    continuerAffichage = false;
                    webViewSalaireChangeant.clearCache(false);
                }

            };
            threadMajSalaire.start(); // Lancement du thread
            
        } else if(buttonAutresApps.equals(v)) { // Clic sur le bouton des autres applications
            try {
                startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("market://search?q=pub:droideilhan")));
            } catch(ActivityNotFoundException e) {
                showMessageDialog(R.string.texte_erreur_market);
            }
        }
        
        if(!v.equals(editTextSalaire)) { // Si on clique ailleurs que sur l'edittext du salaire
            // On cache le clavier
            InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.hideSoftInputFromWindow(editTextSalaire.getWindowToken(), 0);
        }
    }

    /**
     * Charge les statistiques USD du salaire
     */
    private void loadStatistiquesUSD(float salaireParJour, float salaireParHeure, float salaireParMinute, float salaireParSeconde) {
        textViewVotreSalaire.setVisibility(View.VISIBLE);
        textViewSalaireJour.setVisibility(View.VISIBLE);
        webViewSalaireJour.setVisibility(View.VISIBLE);
        webViewSalaireJour.loadDataWithBaseURL("file:///android_asset/", getHtmlDataFromMontantUSD(salaireParJour), "text/html", "UTF-8", "");
        textViewSalaireHeure.setVisibility(View.VISIBLE);
        webViewSalaireHeure.setVisibility(View.VISIBLE);
        webViewSalaireHeure.loadDataWithBaseURL("file:///android_asset/", getHtmlDataFromMontantUSD(salaireParHeure), "text/html", "UTF-8", "");
        textViewSalaireMinute.setVisibility(View.VISIBLE);
        webViewSalaireMinute.setVisibility(View.VISIBLE);
        webViewSalaireMinute.loadDataWithBaseURL("file:///android_asset/", getHtmlDataFromMontantUSD(salaireParMinute), "text/html", "UTF-8", "");
        textViewSalaireSeconde.setVisibility(View.VISIBLE);
        webViewSalaireSeconde.setVisibility(View.VISIBLE);
        webViewSalaireSeconde.loadDataWithBaseURL("file:///android_asset/", getHtmlDataFromMontantUSD(salaireParSeconde), "text/html", "UTF-8", "");
        textViewAvantStats.setVisibility(View.GONE);
    }

    /**
     * Charge les statistiques EUR du salaire
     */
    private void loadStatistiquesEUR(float salaireParJour, float salaireParHeure, float salaireParMinute, float salaireParSeconde) {
        textViewVotreSalaire.setVisibility(View.VISIBLE);
        textViewSalaireJour.setVisibility(View.VISIBLE);
        webViewSalaireJour.setVisibility(View.VISIBLE);
        webViewSalaireJour.loadDataWithBaseURL("file:///android_asset/", getHtmlDataFromMontantEUR(salaireParJour), "text/html", "UTF-8", "");
        textViewSalaireHeure.setVisibility(View.VISIBLE);
        webViewSalaireHeure.setVisibility(View.VISIBLE);
        webViewSalaireHeure.loadDataWithBaseURL("file:///android_asset/", getHtmlDataFromMontantEUR(salaireParHeure), "text/html", "UTF-8", "");
        textViewSalaireMinute.setVisibility(View.VISIBLE);
        webViewSalaireMinute.setVisibility(View.VISIBLE);
        webViewSalaireMinute.loadDataWithBaseURL("file:///android_asset/", getHtmlDataFromMontantEUR(salaireParMinute), "text/html", "UTF-8", "");
        textViewSalaireSeconde.setVisibility(View.VISIBLE);
        webViewSalaireSeconde.setVisibility(View.VISIBLE);
        webViewSalaireSeconde.loadDataWithBaseURL("file:///android_asset/", getHtmlDataFromMontantEUR(salaireParSeconde), "text/html", "UTF-8", "");
        textViewAvantStats.setVisibility(View.GONE);
    }
    
    /**
     * Affiche une boite de dialogue du message identifié par messageId
     * @param messageId identifiant du message à afficher
     */
    private void showMessageDialog(int messageId) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(messageId)
               .setCancelable(false)
               .setNeutralButton("OK", new DialogInterface.OnClickListener() {
                   public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                   }
               });
        AlertDialog alert = builder.create();
        alert.show();
    }
    
    /**
     * Retourne les données HTML correspondant au montant (en dollars US) en paramètres
     * @param montant le montant (en dollars US) à afficher
     * @return les données HTML correspondant au montant (en dollars US) en paramètres
     */
    private String getHtmlDataFromMontantUSD(float montant) {
        // Affichage du montant
        StringBuilder htmlData = new StringBuilder("<div align=\"center\"><strong style=\"color: #fff;\">");
        if(montant >= 1.00) {
            int r = (int) Math.floor(montant);
            String k = kFormat((montant - r));
            htmlData.append(r+" dollars "+k+" cents");        
        } else {        
            htmlData.append(kFormat(montant)+" cents");
        }
        htmlData.append("</strong><br />");
        
        // Affichage de la monnaie
        double[] moneys = {100, 50, 20, 10, 5, 2, 1, 0.50, 0.25, 0.10, 0.05, 0.01};
        double[] amounts = new double[moneys.length];

        float montantRestant = montant;
        for(int i = 0; i < moneys.length; i++) {
            double a = Math.floor(montantRestant / moneys[i]);
            amounts[i] = a;
            if(montantRestant >= a * moneys[i]) {
                montantRestant = (float) (montantRestant - a * moneys[i]);
            }
        }

        for(int i = 0; i < amounts.length; i++) {
            for(int j = 0; j < amounts[i]; j++) {
                htmlData.append("<img src=\"monnaie/usd/"+moneys[i]+".png\" /> ");
            }
        }        
        htmlData.append("</div>");
        return htmlData.toString();
    }

    /**
     * Retourne les données HTML correspondant au montant (en euros) en paramètres
     * @param montant le montant (en euros) à afficher
     * @return les données HTML correspondant au montant (en euros) en paramètres
     */
    private String getHtmlDataFromMontantEUR(float montant) {
        // Affichage du montant
        StringBuilder htmlData = new StringBuilder("<div align=\"center\"><strong style=\"color: #fff;\">");
        if(montant >= 1.00) {
            int r = (int) Math.floor(montant);
            String k = kFormat((montant - r));
            htmlData.append(r+" euros "+k+" cents");        
        } else {        
            htmlData.append(kFormat(montant)+" cents");
        }
        htmlData.append("</strong><br />");
        
        // Affichage de la monnaie
        double[] moneys = {500, 200, 100, 50, 20, 10, 5, 2, 1, 0.50, 0.20, 0.10, 0.05, 0.02, 0.01};
        double[] amounts = new double[moneys.length];

        float montantRestant = montant;
        for(int i = 0; i < moneys.length; i++) {
            double a = Math.floor(montantRestant / moneys[i]);
            amounts[i] = a;
            if(montantRestant >= a * moneys[i]) {
                montantRestant = (float) (montantRestant - a * moneys[i]);
            }
        }

        for(int i = 0; i < amounts.length; i++) {
            for(int j = 0; j < amounts[i]; j++) {
                htmlData.append("<img src=\"monnaie/euro/"+moneys[i]+".png\" /> ");
            }
        }        
        htmlData.append("</div>");
        return htmlData.toString();
    }
        
    /**
     * Formate le montant en paramètre
     * @param k
     * @return le montant formaté
     */
    private String kFormat(double k) {
        int k1 = (int) Math.floor(k*100);
        int k2 = (int) Math.floor((k*100-k1)*100); 
        return ((k1 < 10.0)?"0"+k1:k1)+",<small>"+((k2 < 10.0)?"0"+k2:k2)+"</small>";
    }

}
