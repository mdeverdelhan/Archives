package com.android.myfirstapp;

import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.LinearLayout;
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
    }

	@Override
	protected boolean isRouteDisplayed() {
		// TODO Auto-generated method stub
		return false;
	}	
}