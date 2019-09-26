# 3RScan

3RScan is a large scale, real-world dataset which features 1482 3D reconstructions / snapshots of 478 naturally changing indoor environments, designed for benchmarking emerging tasks such as long-term SLAM, scene change detection and object instance re-localization

![example](data/teaser.png)

Each sequence comes with aligned semantically annotated 3D data and corresponding 2D frames, containing in detail:

* calibrated RGB-D sequences.
* textured 3D meshes.
* 6DoF camera poses and camera calibration parameters K.
* global alignment among scans from the same scene as a global transformation T.
* dense instance-level semantic segmentation where each instance has a fixed ID that is kept consistent across different sequences of the same environment.
* object alignment, i.e. a ground truth transformation for each changed object together with its symmetry property.

### Paper
If you find the data useful please consider citing our [paper](https://arxiv.org/pdf/1908.06109.pdf):

```
@inproceedings{Wald2019RIO,
    title={RIO: 3D Object Instance Re-Localization in Changing Indoor Environments},
    author={Johanna Wald, Armen Avetisyan, Nassir Navab, Federico Tombari, Matthias Niessner},
    booktitle={Proceedings IEEE International Conference on Computer Vision (ICCV)},
    year = {2019}
}
```

## Data Organization

The data in 3RScan is organized by RGB-D sequence. Each sequence has a unique hash value to identify the scan. The RGB-sequences and 3D reconstructions are all stored together in a separate folder. The directory has the following structure:

```
<scanId>
|-- mesh.refined.obj
    Reconstructed mesh
|-- mesh.refined.mtl
    Corresponding material file 
|-- mesh.refined_0.png
    Corresponding mesh texture
|-- sequence.zip
    Calibrated RGB-D sensor stream with color and depth frames, camera poses
|-- labels.instances.annotated.ply
    Visualization of semantic segmentation
|-- mesh.refined.0.010000.segs.json
    Over-segmentation of annotation mesh
|-- semseg.json
    Instance segmentation of the mesh (contains the labels)
```

## Data Formats

The following are overviews of the data formats used in 3RScan:

**Reconstructed surface mesh file (`*.obj`)**:
OBJ format mesh with +Z axis in upright orientation.

**RGB-D sensor data (`*.zip`)**:
ZIP-archive with per-frame color, depth, camera pose and camera intrinsics.

**Surface mesh segmentation file (`*.segs.json`)**:
```javascript
{
  "params": {  // segmentation parameters
   "kThresh": "0.0001",
   "segMinVerts": "20",
   "minPoints": "750",
   "maxPoints": "30000",
   "thinThresh": "0.05",
   "flatThresh": "0.001",
   "minLength": "0.02",
   "maxLength": "1"
  },
  "sceneId": "...",  // id of segmented scene
  "segIndices": [1,1,1,1,3,3,15,15,15,15],  // per-vertex index of mesh segment
}
```

**Aggregated semantic annotation file (`*semseg.json`)**:
```javascript
{
  "sceneId": "...",  // id of annotated scene
  "appId": "...", // id + version of the tool used to create the annotation
  "segGroups": [
    {
      "id": 0,
      "objectId": 0,
      "segments": [1,4,3],
      "label": "couch"
    },
  ],
  "segmentsFile": "..." // id of the *.segs.json segmentation file referenced
}
```

**meta data file (`3RScan.json`)**:

```javascript
[
  {
    "reference": "531cff08-0021-28f6-8e08-ba2eeb945e09", // id of the initial scan
    "type": "train"    
    "ambiguity": [
      [
        { "instance_source": 34, 
          "instance_target": 35,
          "transform": [ ... ]  // transformation of instance 34 to instance 35 (to resolve instance ambiguity)
        }, {...}
      ]
    ],
    "scans": [ // rescans
      {
        "reference": "531cff10-0021-28f6-8f94-80db8fdbbbee", // id of rescan
        "transform": [ ... ]  // transformation to align rescan with reference
        "nonrigid": [ ... ],  // list of instances with nonrigid changes
        "removed": [], // removed instances
        "rigid": [ // rigid changes
          {
            "instance_reference": 35, // instance ID in reference
            "instance_rescan": 35, // instance ID in rescan
            "symmetry": 0, // symmetry (0 = none)
            "transform": [ ... ] // transformation to align instance in the reference to the instance in the rescan
          }, {
            ...
          }         
        ],
        
      },
      // metadata of other rescans
      {
        "reference": "19eda6f4-55aa-29a0-8893-8eac3a4d8193", ... 
      }, { ... }
    ]
  }
]
```

## Training Data

You can find the train, test and validation splits here: [[train](splits/train.txt), [val](splits/val.txt), [test](splits/test.txt)]

### Notes:
* segmentation format (surface mesh segmentation and aggregated semantic annotation) is the same as in [ScanNet](https://github.com/ScanNet/ScanNet)
* Please see our [project page](https://waldjohannau.github.io/RIO) for more information.

