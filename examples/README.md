# 📚 Example Terrain Graphs

Pre-made terrain graphs demonstrating different techniques and workflows.

---

## 🎓 How to Use Examples

1. **Load in App**: `File → Open Graph → Browse to examples/ folder`
2. **Execute**: Click `Graph → Execute` in menu bar
3. **View Result**: See terrain in 3D Viewport
4. **Modify**: Adjust parameters and re-execute
5. **Export**: `File → Export Heightmap` or `Export Mesh`

---

## 📁 Available Examples

### 01_simple_perlin.tgraph
**Difficulty**: Beginner
**Nodes**: 2 (Perlin + Output)
**Description**: Basic Perlin noise terrain - smooth, natural heightfield

**Use Cases**:
- Gentle rolling hills
- Valleys and plains
- Base layer for more complex terrains

**Parameters to Experiment With**:
- `frequency`: Higher = more detailed (try 0.005 - 0.02)
- `octaves`: More octaves = more detail (try 4-8)
- `amplitude`: Controls height (try 0.5 - 2.0)

---

### 02_eroded_mountains.tgraph
**Difficulty**: Intermediate
**Nodes**: 4 (Perlin → Hydraulic → Thermal → Output)
**Description**: Realistic mountains with water erosion valleys and gravity erosion

**Use Cases**:
- Natural mountain ranges
- River valley networks
- Realistic aged terrain

**Key Concepts**:
- **Hydraulic Erosion**: Creates valleys where water would flow
- **Thermal Erosion**: Smooths steep slopes via gravity
- **Chaining**: Output of one node feeds into next

**Performance**: ~5-10 seconds on modern GPU (2000+ iterations)

---

### 03_rocky_peaks.tgraph
**Difficulty**: Intermediate
**Nodes**: 5 (Ridged + Voronoi → Add → Thermal → Output)
**Description**: Sharp, dramatic peaks using multiple noise types

**Use Cases**:
- Alpine mountain peaks
- Rocky, jagged terrain
- Fantasy landscapes

**Key Concepts**:
- **Ridged Noise**: Creates sharp mountain ridges
- **Voronoi Cells**: Adds rocky texture
- **Add Node**: Combines two heightfields with weighting
- **Thermal Erosion**: Sharpens features

---

### 04_desert_dunes.tgraph *(Coming Soon)*
**Difficulty**: Intermediate
**Description**: Rolling sand dunes with wind-driven patterns

---

### 05_volcanic_island.tgraph *(Coming Soon)*
**Difficulty**: Advanced
**Description**: Island with central volcano, beaches, and slopes

---

### 06_canyon_system.tgraph *(Coming Soon)*
**Difficulty**: Advanced
**Description**: Deep canyons and mesas using advanced erosion

---

## 🎨 Creating Your Own Graphs

### Recommended Workflow

1. **Start Simple**: Begin with one generator node
2. **Add Detail**: Chain modifiers and filters
3. **Add Realism**: Apply erosion
4. **Fine-tune**: Adjust parameters iteratively
5. **Save Often**: `File → Save Graph`

### Common Node Combinations

#### Natural Terrain
```
Perlin Noise → Hydraulic Erosion → Thermal Erosion → Output
```

#### Rocky Mountains
```
Ridged Noise → Thermal Erosion → Output
```

#### Textured Hills
```
Perlin (large scale) + Voronoi (small scale) → Add → Output
```

#### Smooth Valleys
```
Perlin → Gaussian Blur → Output
```

---

## 💡 Tips & Tricks

### Resolution Guidelines
- **Prototyping**: 256x256 or 512x512 (fast iteration)
- **Final Output**: 1024x1024 or 2048x2048
- **High Detail**: 4096x4096 (slow, for final renders)

### Erosion Settings
- **Light Erosion**: 500-1000 iterations
- **Medium Erosion**: 1000-2000 iterations
- **Heavy Erosion**: 2000-5000 iterations (slow!)

### Frequency vs. Amplitude
- **Low Frequency (0.001-0.01)**: Large-scale features (mountains, valleys)
- **High Frequency (0.02-0.1)**: Small-scale details (bumps, texture)
- **Low Amplitude (0.1-0.5)**: Gentle terrain
- **High Amplitude (1.0-2.0)**: Dramatic terrain

---

## 🐛 Troubleshooting Examples

### "Graph fails to execute"
- **Check connections**: Ensure all nodes are connected
- **Check parameters**: Some parameters have valid ranges
- **Check output node**: Must have one Output node

### "Terrain looks flat"
- **Increase amplitude**: Try doubling the amplitude parameter
- **Increase octaves**: More octaves = more detail
- **Check height scale**: In export settings

### "Erosion too slow"
- **Reduce iterations**: Start with 500-1000
- **Reduce resolution**: Use 256x256 for testing
- **Enable GPU**: Ensure Vulkan is available

---

## 📖 Learning Path

1. **Beginner**: Load `01_simple_perlin.tgraph`
   - Adjust frequency and amplitude
   - Learn basic parameters

2. **Intermediate**: Load `02_eroded_mountains.tgraph`
   - Understand erosion effects
   - Learn node chaining

3. **Advanced**: Load `03_rocky_peaks.tgraph`
   - Combine multiple generators
   - Use combiner nodes

4. **Expert**: Create your own from scratch
   - 5+ nodes
   - Multiple erosion passes
   - Custom parameters

---

## 🎯 Challenges

Try recreating these terrains:

### Challenge 1: **Grand Canyon**
- Deep valleys
- Layered rock formations
- Flat mesas

**Hints**: Heavy hydraulic erosion, voronoi for layers

### Challenge 2: **Himalayas**
- Extremely sharp peaks
- Deep valleys
- Snow-capped appearance

**Hints**: Ridged noise, high amplitude, thermal erosion

### Challenge 3: **Scottish Highlands**
- Rolling hills
- Gentle slopes
- Weathered appearance

**Hints**: Low frequency perlin, medium hydraulic erosion

---

## 📚 Additional Resources

- **User Manual**: See `USER_GUIDE.md`
- **Node Reference**: See `docs/NODE_REFERENCE.md`
- **Video Tutorials**: [Link coming soon]
- **Community Graphs**: [Link to sharing platform]

---

**Have a great example graph?**
Share it! Submit via GitHub PR to `examples/` folder.

*Last Updated: November 2025*
