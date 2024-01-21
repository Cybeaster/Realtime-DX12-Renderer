


// Helper functions to determine LOD and subdivide the icosahedron
int DetermineLOD(float distance)
{
	if (distance < 50)
        return 4; // Subdivide 4 times
    else if (distance < 100)
        return 3; // Subdivide 3 times
    else if (distance < 200)
        return 2; // Subdivide 2 times
    else
        return 1; // Subdivide 1 time
}

