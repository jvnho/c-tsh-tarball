
public class Noeud {
	private int etiquette;
	private Noeud gauche;
	private Noeud droit;
	
	public Noeud (int etiquette, Noeud g, Noeud d) {
		this.etiquette = etiquette;
		this.gauche = g;
		this.droit = d;
	}
	public Noeud(int etiquette) {
		this(etiquette,null,null);
	}
	
	public Noeud(Noeud arbre) {
		this.etiquette = arbre.etiquette;
		if(arbre.gauche != null) 
			this.gauche = new Noeud(arbre.gauche);
		if(arbre.droit != null) 
			this.droit = new Noeud(arbre.droit);
	}

	public void afficheInfixe() {
		if(this.gauche != null)
			this.gauche.afficheInfixe();
		System.out.print(this.etiquette + " ");
		if(this.droit != null)
			this.droit.afficheInfixe();
	}
	public void affichePostfixe() {
		if(this.gauche != null)
			this.gauche.affichePostfixe();
		if(this.droit != null)
			this.droit.affichePostfixe();
		System.out.print(this.etiquette + " ");
	}
	public void affichePrefixe() {
		System.out.print(this.etiquette + " ");
		if(this.gauche != null)
			this.gauche.affichePrefixe();
		if(this.droit != null)
			this.droit.affichePrefixe();
	}
	
	public int nbDeNoeuds() {
		int ret = 1;
		if(this.gauche != null)
			ret +=this.gauche.nbDeNoeuds();
		if(this.droit != null) 
			ret += this.droit.nbDeNoeuds();
		return ret;
	}
	
	public int somme(){
		int res = this.etiquette;
		if(this.gauche != null)
			res +=this.gauche.somme();
		if(this.droit != null)
			res +=this.droit.somme();
		return res;
	}
	
	public int profondeur() {
		int ret = 0;
		int ret2 = 0;
		if(this.gauche != null) {
			ret = 1 + this.gauche.profondeur();
		}
		if(this.droit != null) {
			ret2 = 1 + this.droit.profondeur();
			if(ret < ret2)
				ret = ret2;
		}
		return ret;
	}
	
	public boolean recherche(int e) {
		if(this.etiquette == e)
			return true;
		if((this.gauche != null && this.gauche.recherche(e))|| (this.droit != null && this.droit.recherche(e)))
			return true;
		return false;
	}
}
