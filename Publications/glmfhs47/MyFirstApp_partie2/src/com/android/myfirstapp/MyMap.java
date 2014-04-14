package com.android.myfirstapp;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;

import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import com.google.android.maps.GeoPoint;
import com.google.android.maps.MapActivity;
import com.google.android.maps.MapController;
import com.google.android.maps.MapView;

public class MyMap extends MapActivity {

	private MapView mapView; /** Vue de la carte */
    private MapController mc; /** Controleur de la carte */
    private int zoomInitial; /** Valeur initiale du zoom */
    private GeoPoint centreInitial; /** Centre de la carte */
        
    /**
     * 
     */
    private URL buildUrl(double latitude, double longitude, int rayon) {
    	// Construction de l'URL du service Web d'Ecoemballages
		URL url = null;
		try {
    		StringBuffer bufferUrl = new StringBuffer("http://tri-recyclage.ecoemballages.fr/googlemaps/find-decheteries.php?");
    		bufferUrl.append("lat="+latitude);
    		bufferUrl.append("&lng="+longitude);
    		bufferUrl.append("&radius="+rayon*0.62137119223733395);

			url = new URL(bufferUrl.toString());
		} catch (MalformedURLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return url;
    }
    
    /**
     * Retourne la liste des déchèteries après parsing d'ecoemballages
     * @param url URL à parser
     * @return une liste de markers tirée de l'URL
     */
    private ListeMarkers parseXml(URL url) {
    	// Création du parseur SAX
        SAXParserFactory spf = SAXParserFactory.newInstance();
        SAXParser sp = null;
		try {
			sp = spf.newSAXParser();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

        // Récupération du lecteur XML du parseur SAX nouvellement créé
        XMLReader xr = null;
		try {
			xr = sp.getXMLReader();
		} catch (SAXException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
        /* Create a new ContentHandler and apply it to the XML-Reader*/
        SAXEventHandler saxEventHandler = new SAXEventHandler(this, this.getResources().getDrawable(R.drawable.marker_decheterie));
        xr.setContentHandler(saxEventHandler);
        
        // Début du parsing du contenu de l'URL
        InputSource in = new InputSource();            
		try {
			// Récupération du contenu de l'URL
            in.setByteStream(url.openStream());
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        //in.setEncoding("ISO-8859-1"); // Changement du charset
        try {
			xr.parse(in); // Parsing
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        // Fin du parsing
        return saxEventHandler.getListeDesMarkers();
    }
    
	/** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);		
        setContentView(R.layout.map);
        mapView = (MapView) findViewById(R.id.gmapview);
        mc = mapView.getController();
        
        // Récupération des paramètres
        Bundle bundle = this.getIntent().getExtras();
        double latitude = bundle.getDouble("LATITUDE");
        double longitude = bundle.getDouble("LONGITUDE");
        int rayon = bundle.getInt("RAYON");
        
        zoomInitial = 14;
        centreInitial = new GeoPoint((int) (latitude * 1E6), 
        							(int) (longitude * 1E6));
        
        mc.animateTo(centreInitial);
        mc.setZoom(zoomInitial);
        mapView.setSatellite(true);
        mapView.setBuiltInZoomControls(true);
        //mapView.invalidate();
        Toast.makeText(this, 
				"Lat : "+latitude+" Lng : "+longitude+" Ray : "+rayon, 
				Toast.LENGTH_LONG).show();

		URL url = buildUrl(latitude, longitude, rayon);
        ListeMarkers listeDesMarkers = parseXml(url);
        mapView.getOverlays().add(listeDesMarkers);
        
        YouAreHereOverlay youAreHereOverlay = new YouAreHereOverlay(this.getResources().getDrawable(R.drawable.blue_marker));
        youAreHereOverlay.setYouAreHereItem(new Marker("You are here!", "Vous êtes ici!", latitude+"", longitude+""));
        mapView.getOverlays().add(youAreHereOverlay);
    }

	@Override
	protected boolean isRouteDisplayed() {
		// TODO Auto-generated method stub
		return false;
	}	
}