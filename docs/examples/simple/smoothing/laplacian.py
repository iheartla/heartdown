"""
The cotangent Laplacian of a triangle mesh, as igl.cotmatrix computes it,
for running the figure where libigl isn't available (in the browser)
"""
import numpy as np
import scipy.sparse


def cotmatrix(V, F):
    """
    :param V: #V×3 vertex positions
    :param F: #F×3 vertex indices of the triangles
    :return: #V×#V sparse L, negative semidefinite: L_ij = (cot α_ij + cot β_ij) / 2, L_ii = -∑_j L_ij
    """
    V = np.asarray(V, dtype=float)
    F = np.asarray(F)
    rows, cols, values = [], [], []
    for k in range(3):
        # the edge (i, j) and the cotangent of the angle opposite it, at o
        o, i, j = F[:, k], F[:, (k + 1) % 3], F[:, (k + 2) % 3]
        a, b = V[i] - V[o], V[j] - V[o]
        cot = (a * b).sum(axis=1) / np.linalg.norm(np.cross(a, b), axis=1)
        rows += [i, j]
        cols += [j, i]
        values += [cot / 2, cot / 2]
    n = len(V)
    L = scipy.sparse.coo_matrix((np.concatenate(values), (np.concatenate(rows), np.concatenate(cols))), shape=(n, n)).tocsr()
    return L - scipy.sparse.diags(np.asarray(L.sum(axis=1)).ravel())
