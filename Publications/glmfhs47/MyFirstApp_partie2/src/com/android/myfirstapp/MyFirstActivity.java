package com.android.myfirstapp;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public class MyFirstActivity extends Activity {
	
    private LocationManager lm; /** Location manager */
    /** Boutons */
    private Button btn10km;
    private Button btn30km;
    private Button btn50km;
	
    private class MyLocationListener
			implements LocationListener {

		public void onLocationChanged(Location location) {
			// TODO Auto-generated method stub
			
		}

		public void onProviderDisabled(String provider) {
			// TODO Auto-generated method stub
			
		}

		public void onProviderEnabled(String provider) {
			// TODO Auto-generated method stub
			
		}

		public void onStatusChanged(String provider, int status, Bundle extras) {
			// TODO Auto-generated method stub
			
		}
    	
    }
    
    /** Listener sur les boutons */
    private OnClickListener	btnListener = new OnClickListener()
    {
    	/**
    	 * @param v "vue" sur laquelle on a cliqué
    	 */
		public void onClick(View arg0) {
			/**
			 *  Récupération de la position depuis 
			 *  le location provider GPS
			 */
			Location loc;
			loc = MyFirstActivity.this.lm.getLastKnownLocation("gps");
    		if(loc == null) {
    			/**
    			 *  Pas de GPS ?
    			 *  Récupération de la position depuis 
    			 *  le location provider réseau
    			 */
    			loc = MyFirstActivity.this.lm.getLastKnownLocation("network");
    		}
    		if(loc != null) {
	    		// Lyonnais
	    		//loc.setLatitude(45.807502);
	    		//loc.setLongitude(4.806687);
    			/**
    			 *  Lancement de l'activité MyMap et passage des
    			 *  paramètres
    			 */
	    		Intent intent;
	    		intent = new Intent(MyFirstActivity.this, MyMap.class);
	    		Bundle bundle = new Bundle();
	    		bundle.putDouble("LATITUDE", loc.getLatitude());
	    		bundle.putDouble("LONGITUDE", loc.getLongitude());
	    		bundle.putInt("RAYON", getRayon(arg0));
	    		intent.putExtras(bundle);
	    		startActivity(intent);
    		} else {
    			// Impossible de récupérer la position
    			Toast.makeText(MyFirstActivity.this, 
    						"Impossible de récupérer la position", 
    						Toast.LENGTH_SHORT).show();
    		}
		}
    };
    
    /**
     * @param btn bouton à partir duquel on recupère le rayon
     * @return le rayon
     */
    private int getRayon(View btn) {
		if(btn10km.equals(btn)) {
			return 10;
		} else if(btn30km.equals(btn)) {
			return 30;
		} else if(btn50km.equals(btn)) {
			return 50;
		} else {
			Log.d("Debug MyFirstApp", "Bouton inattendu");
			return 10; // Rayon par défaut
		}
    }
    
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        // Création des boutons de rayon
        btn10km = (Button) findViewById(R.id.btn10km);
        btn10km.setOnClickListener(btnListener);
        btn30km = (Button) findViewById(R.id.btn30km);
        btn30km.setOnClickListener(btnListener);
        btn50km = (Button) findViewById(R.id.btn50km);
        btn50km.setOnClickListener(btnListener);

        // Récupération du LocationManager
		this.lm = (LocationManager)
				getSystemService(Context.LOCATION_SERVICE);
        this.lm.requestLocationUpdates(
        		"gps", 0, 0, new MyLocationListener());
    }
}