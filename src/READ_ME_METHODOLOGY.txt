Utilité du fichier: écrire la méthodologie




- créer une logique producer consumer entre main thread and worker threads

- créer buffer --> LE buffer se remplit quand il n'y a worker threads de disponible 
		( thread should place the connection descriptor into a fixed-size buffer and
		return to accepting more connections.)