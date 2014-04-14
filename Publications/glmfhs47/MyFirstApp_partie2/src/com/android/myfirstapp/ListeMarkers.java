package com.android.myfirstapp;

import java.util.ArrayList;

import android.app.Dialog;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.view.Window;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.maps.ItemizedOverlay;

public class ListeMarkers extends ItemizedOverlay<Marker> {
	
	private ArrayList<Marker> listeDesMarkers = new ArrayList<Marker>();
	
	private Context context;
	
	public ListeMarkers(Context context, Drawable defaultMarker) {
		super(boundCenterBottom(defaultMarker));
		this.context = context;
		populate();
	}
	
	/**
	 * Ajoute un marker à la liste
	 * @param m le marker à ajouter
	 */
	public void addMarker(Marker m) {
		listeDesMarkers.add(m);
		populate();
	}
	
	/** @return string représentant la liste des markers */
    public String toString() {
		String nline = System.getProperty("line.separator");
		StringBuffer buf = new StringBuffer();
		
		buf.append( "--- Markers ---" ).append(nline);
		for(int i = 0; i < listeDesMarkers.size(); i++){
		    buf.append(listeDesMarkers.get(i)).append(nline);
		}
		return buf.toString();
    }

	@Override
	protected Marker createItem(int i) {
		return listeDesMarkers.get(i);
	}

	@Override
	public int size() {
		return listeDesMarkers.size();
	}
	
	@Override
	protected boolean onTap(int index) {
		//com.android.myfirstapp.
		// Création de la boite de dialogue
		final Dialog dialog = new Dialog(context);
		dialog.requestWindowFeature(Window.FEATURE_NO_TITLE); // Suppression de la zone de titre pour les Dialog des déchèteries
		dialog.setContentView(R.layout.dialog);
		dialog.setCancelable(true);
		dialog.setCanceledOnTouchOutside(true);
		
		// Affichage du nom et de l'adresse
		TextView viewNom = (TextView) dialog.findViewById(R.id.nom);
		viewNom.setText(listeDesMarkers.get(index).getNom());
		TextView viewAdresse = (TextView) dialog.findViewById(R.id.adresse);
		viewAdresse.setText(listeDesMarkers.get(index).getAdresse());
		
		dialog.show();
		
		return super.onTap(index);
	}
}