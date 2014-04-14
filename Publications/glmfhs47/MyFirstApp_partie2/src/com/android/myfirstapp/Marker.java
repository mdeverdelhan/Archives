package com.android.myfirstapp;

import com.google.android.maps.GeoPoint;
import com.google.android.maps.OverlayItem;

public class Marker extends OverlayItem {
	
	/** Attributs de l'élément XML marker */
	private String nom;
	private String adresse;
	private String lat;
	private String lng;
	
	public Marker(String nom, String adresse, String lat, String lng) {
		super(new GeoPoint((int) (Double.parseDouble(lat) * 1E6), (int) (Double.parseDouble(lng) * 1E6)), nom, nom+"\n"+adresse);
		this.nom = nom;
		this.adresse = adresse;
		this.lat = lat;
		this.lng = lng;
	}
	
	/** @return the nom */
	public String getNom() {
		return nom;
	}

	/**
	 * @return the adresse
	 */
	public String getAdresse() {
		return adresse;
	}

	/**
	 * @return the lat
	 */
	public String getLat() {
		return lat;
	}

	/**
	 * @return the lng
	 */
	public String getLng() {
		return lng;
	}

	/**
	 * @param nom the nom to set
	 */
	public void setNom(String nom) {
		this.nom = nom;
	}

	/**
	 * @param adresse the adresse to set
	 */
	public void setAdresse(String adresse) {
		this.adresse = adresse;
	}

	/**
	 * @param lat the lat to set
	 */
	public void setLat(String lat) {
		this.lat = lat;
	}

	/**
	 * @param lng the lng to set
	 */
	public void setLng(String lng) {
		this.lng = lng;
	}

	/** @return une string représentant un marker */
	public String toString() {
		StringBuffer buf = new StringBuffer();
		buf.append("Marker: Nom='" + this.nom + "', ");
		buf.append("Adresse='" + this.adresse + "', ");
		buf.append("Lat='" + this.lat + "', ");
		buf.append("Lng='" + this.lng + "'");
		return buf.toString();
						
	}
}