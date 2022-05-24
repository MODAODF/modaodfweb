// OverlayTransform is used by CanvasOverlay to apply transformations
// to points/bounds before drawing is done.
// The reason why we cannot use canvasRenderingContext2D.transform() is it
// does not support coordinate values bigger than 2^24 - 1 and if we use it in this
// regime the renders will be incorrect. At least in Calc it is possible to have pixel
// coordinates greater than this limit at higher zooms near the bottom of the sheet.
class OverlayTransform {
	private translationAmount: oxool.Point;
	private scaleAmount: oxool.Point;

	// This does not support a stack of transforms and hence only allows a single
	// scaling and translation operation.
	constructor() {
		this.translationAmount = new oxool.Point(0, 0);
		this.scaleAmount = new oxool.Point(1, 1);
	}

	translate(x: number, y: number) {
		this.translationAmount.x = x;
		this.translationAmount.y = y;
	}

	scale(sx: number, sy: number) {
		this.scaleAmount.x = sx;
		this.scaleAmount.y = sy;
	}

	reset() {
		this.translationAmount.x = 0;
		this.translationAmount.y = 0;
		this.scaleAmount.x = 1;
		this.scaleAmount.y = 1;
	}

	applyToPoint(point: oxool.Point): oxool.Point {
		// 'scale first then translation' model.
		return new oxool.Point(
			point.x * this.scaleAmount.x - this.translationAmount.x,
			point.y * this.scaleAmount.y - this.translationAmount.y);
	}

	applyToBounds(bounds: oxool.Bounds): oxool.Bounds {
		return new oxool.Bounds(
			this.applyToPoint(bounds.min),
			this.applyToPoint(bounds.max)
		);
	}
}

// CanvasOverlay handles CPath rendering and mouse events handling via overlay-section of the main canvas.
// where overlays like cell-cursors, cell-selections, edit-cursors are instances of CPath or its subclasses.
class CanvasOverlay {
	private map: any;
	private ctx: CanvasRenderingContext2D;
	private paths: Map<number, any>;
	private bounds: oxool.Bounds;
	private tsManager: any;
	private overlaySection: any;
	private transform: OverlayTransform;

	constructor(mapObject: any, canvasContext: CanvasRenderingContext2D) {
		this.map = mapObject;
		this.ctx = canvasContext;
		this.tsManager = this.map.getTileSectionMgr();
		this.overlaySection = undefined;
		this.paths = new Map<number, CPath>();
		this.transform = new OverlayTransform();
		this.updateCanvasBounds();
	}

	onInitialize() {
		return;
	}

	onResize() {
		this.paths.forEach(function (path: CPath) {
			path.onResize();
		});
		this.onDraw();
	}

	onDraw() {
		// No need to "erase" previous drawings because tiles are draw first via its onDraw.
		this.draw();
	}

	onMouseMove(position: Array<number>) {
		var mousePos = new oxool.Point(position[0], position[1]);
		var overlaySectionBounds = this.bounds.clone();
		var splitPos = this.tsManager.getSplitPos();
		if (mousePos.x > splitPos.x)
			mousePos.x += overlaySectionBounds.min.x;
		if (mousePos.y > splitPos.y)
			mousePos.y += overlaySectionBounds.min.y;

		this.paths.forEach(function (path:CPath) {
			var pathBounds = path.getBounds();

			if (!pathBounds.isValid())
				return;

			var mouseOverPath = pathBounds.contains(mousePos);
			if (mouseOverPath && !path.isUnderMouse()) {
				path.onMouseEnter(mousePos);
				path.setUnderMouse(true);
			} else if (!mouseOverPath && path.isUnderMouse()) {
				path.onMouseLeave(mousePos);
				path.setUnderMouse(false);
			}
		});
	}

	setOverlaySection(overlaySection: any) {
		this.overlaySection = overlaySection;
	}

	getTestDiv(): HTMLDivElement {
		return this.overlaySection.getTestDiv();
	}

	setPenOnOverlay() {
		this.overlaySection.containerObject.setPenPosition(this.overlaySection);
	}

	initPath(path: CPath) {
		var pathId: number = path.getId();
		this.paths.set(pathId, path);
		path.setRenderer(this);
		this.setPenOnOverlay();
		path.updatePathAllPanes();
	}

	initPathGroup(pathGroup: CPathGroup) {
		pathGroup.forEach(function (path: CPath) {
			this.initPath(path);
		}.bind(this));
	}

	removePath(path: CPath) {
		// This does not get called via onDraw, so ask section container to redraw everything.
		path.setDeleted();
		this.paths.delete(path.getId());
		this.overlaySection.containerObject.requestReDraw();
	}

	removePathGroup(pathGroup: CPathGroup) {
		pathGroup.forEach(function (path: CPath) {
			this.removePath(path);
		}.bind(this));
	}

	updatePath(path: CPath, oldBounds: oxool.Bounds) {
		this.redraw(path, oldBounds);
	}

	updateStyle(path: CPath, oldBounds: oxool.Bounds) {
		this.redraw(path, oldBounds);
	}

	paintRegion(paintArea: oxool.Bounds) {
		this.draw(paintArea);
	}

	getSplitPanesContext(): any {
		return this.map.getSplitPanesContext();
	}

	private isVisible(path: CPath): boolean {
		var pathBounds = path.getBounds();
		if (!pathBounds.isValid())
			return false;
		return this.intersectsVisible(pathBounds);
	}

	private intersectsVisible(queryBounds: oxool.Bounds): boolean {
		this.updateCanvasBounds();
		var spc = this.getSplitPanesContext();
		return spc ? spc.intersectsVisible(queryBounds) : this.bounds.intersects(queryBounds);
	}

	private static renderOrderComparator(a: CPath, b: CPath): number {
		if (a.viewId === -1 && b.viewId === -1) {
			// Both are 'own' / 'self' paths.

			// Both paths are part of the same group, use their zindex to break the tie.
			if (a.groupType === b.groupType)
				return a.zIndex - b.zIndex;

			return a.groupType - b.groupType;

		} else if (a.viewId === -1) {
			// a is an 'own' path and b is not => draw a on top of b.
			return 1;

		} else if (b.viewId === -1) {
			// b is an 'own' path and a is not => draw b on top of a.
			return -1;

		}

		// Both a and b belong to other views.

		if (a.viewId === b.viewId) {
			// Both belong to the same view.

			// Both paths are part of the same group, use their zindex to break the tie.
			if (a.groupType === b.groupType)
				return a.zIndex - b.zIndex;

			return a.groupType - b.groupType;

		}

		// a and b belong to different views.
		return a.viewId - b.viewId;
	}

	private draw(paintArea?: oxool.Bounds) {
		if (this.tsManager && this.tsManager.waitForTiles()) {
			// don't paint anything till tiles arrive for new zoom.
			return;
		}

		var orderedPaths = Array<CPath>();
		this.paths.forEach((path: CPath) => {
			orderedPaths.push(path);
		});

		// Sort them w.r.t. rendering order.
		orderedPaths.sort(CanvasOverlay.renderOrderComparator);

		orderedPaths.forEach((path: CPath) => {
			if (this.isVisible(path))
				path.updatePathAllPanes(paintArea);
		}, this);
	}

	private redraw(path: CPath, oldBounds: oxool.Bounds) {
		if (this.tsManager && this.tsManager.waitForTiles()) {
			// don't paint anything till tiles arrive for new zoom.
			return;
		}

		if (!this.isVisible(path) && (!oldBounds.isValid() || !this.intersectsVisible(oldBounds)))
			return;
		// This does not get called via onDraw(ie, tiles aren't painted), so ask tileSection to "erase" by painting over.
		// Repainting the whole canvas is not necessary but finding the minimum area to paint over
		// is potentially expensive to compute (think of overlapped path objects).
		// TODO: We could repaint the area on the canvas occupied by all the visible path-objects
		// and paint tiles just for that, but need a more general version of _tilesSection.onDraw() and callees.
		this.overlaySection.containerObject.requestReDraw();
	}

	private updateCanvasBounds() {
		var viewBounds: any = this.map.getPixelBoundsCore();
		this.bounds = new oxool.Bounds(new oxool.Point(viewBounds.min.x, viewBounds.min.y), new oxool.Point(viewBounds.max.x, viewBounds.max.y));
	}

	getBounds(): oxool.Bounds {
		this.updateCanvasBounds();
		return this.bounds;
	}

	// Applies canvas translation so that polygons/circles can be drawn using core-pixel coordinates.
	private ctStart(clipArea?: oxool.Bounds, paneBounds?: oxool.Bounds, fixed?: boolean) {
		this.updateCanvasBounds();
		this.transform.reset();
		this.ctx.save();

		if (!paneBounds)
			paneBounds = this.bounds.clone();

		if (this.tsManager._inZoomAnim && !fixed) {
			// zoom-animation is in progress : so draw overlay on main canvas
			// at the current frame's zoom level.

			var splitPos = this.tsManager.getSplitPos();
			var scale = this.tsManager._zoomFrameScale;
			var pinchCenter = this.tsManager._newCenter;

			var center = paneBounds.min.clone();
			if (pinchCenter.x >= paneBounds.min.x && pinchCenter.x <= paneBounds.max.x)
				center.x = pinchCenter.x;
			if (pinchCenter.y >= paneBounds.min.y && pinchCenter.y <= paneBounds.max.y)
				center.y = pinchCenter.y;

			var leftMin = paneBounds.min.x < 0 ? -Infinity : 0;
			var topMin = paneBounds.min.y < 0 ? -Infinity : 0;
			// Compute the new top left in core pixels that ties with the origin of overlay canvas section.
			var newTopLeft = new oxool.Point(
				Math.max(leftMin,
					-splitPos.x - 1 + (center.x - (center.x - paneBounds.min.x) / scale)),
				Math.max(topMin,
					-splitPos.y - 1 + (center.y - (center.y - paneBounds.min.y) / scale)));

			// Compute clip area which needs to be applied after setting the transformation.
			var clipTopLeft = new oxool.Point(0, 0);
			// Original pane size.
			var paneSize = paneBounds.getSize();
			var clipSize = paneSize.clone();
			if (paneBounds.min.x || (!paneBounds.min.x && !splitPos.x)) {
				clipTopLeft.x = newTopLeft.x + splitPos.x;
				// Pane's "free" size will shrink(expand) as we zoom in(out)
				// respectively because fixed pane size expand(shrink).
				clipSize.x = (paneSize.x - splitPos.x * (scale - 1)) / scale;
			}
			if (paneBounds.min.y || (!paneBounds.min.y && !splitPos.y)) {
				clipTopLeft.y = newTopLeft.y + splitPos.y;
				// See comment regarding pane width above.
				clipSize.y = (paneSize.y - splitPos.y * (scale - 1)) / scale;
			}
			// Force clip area to the zoom frame area of the pane specified.
			clipArea = new oxool.Bounds(
				clipTopLeft,
				clipTopLeft.add(clipSize));

			this.transform.scale(scale, scale);
			this.transform.translate(scale * newTopLeft.x, scale * newTopLeft.y);

		} else if (this.tsManager._inZoomAnim && fixed) {

			var scale = this.tsManager._zoomFrameScale;
			this.transform.scale(scale, scale);

			if (clipArea) {
				clipArea = new oxool.Bounds(
					clipArea.min.divideBy(scale),
					clipArea.max.divideBy(scale)
				);
			}

		} else {
			this.transform.translate(
				paneBounds.min.x ? this.bounds.min.x : 0,
				paneBounds.min.y ? this.bounds.min.y : 0);
		}

		if (clipArea) {
			this.ctx.beginPath();
			clipArea = this.transform.applyToBounds(clipArea);
			var clipSize = clipArea.getSize();
			this.ctx.rect(clipArea.min.x, clipArea.min.y, clipSize.x, clipSize.y);
			this.ctx.clip();
		}
	}

	// Undo the canvas translation done by ctStart().
	private ctEnd() {
		this.ctx.restore();
	}

	updatePoly(path: CPath, closed: boolean = false, clipArea?: oxool.Bounds, paneBounds?: oxool.Bounds) {
		var i: number;
		var j: number;
		var len2: number;
		var part: oxool.Point;
		var parts = path.getParts();
		var len: number = parts.length;

		if (!len)
			return;

		this.ctStart(clipArea, paneBounds, path.fixed);
		this.ctx.beginPath();

		for (i = 0; i < len; i++) {
			for (j = 0, len2 = parts[i].length; j < len2; j++) {
				part = this.transform.applyToPoint(parts[i][j]);
				this.ctx[j ? 'lineTo' : 'moveTo'](part.x, part.y);
			}
			if (closed) {
				this.ctx.closePath();
			}
		}

		this.fillStroke(path);

		this.ctEnd();
	}

	updateCircle(path: CPath, clipArea?: oxool.Bounds, paneBounds?: oxool.Bounds) {
		if (path.empty())
			return;

		this.ctStart(clipArea, paneBounds, path.fixed);

		var point = this.transform.applyToPoint(path.point);
		var r: number = path.radius;
		var s: number = (path.radiusY || r) / r;

		if (s !== 1) {
			this.ctx.save();
			this.ctx.scale(1, s);
		}

		this.ctx.beginPath();
		this.ctx.arc(point.x, point.y / s, r, 0, Math.PI * 2, false);

		if (s !== 1) {
			this.ctx.restore();
		}

		this.fillStroke(path);

		this.ctEnd();
	}

	private fillStroke(path: CPath) {

		if (path.fill) {
			this.ctx.globalAlpha = path.fillOpacity;
			this.ctx.fillStyle = path.fillColor || path.color;
			this.ctx.fill(path.fillRule || 'evenodd');
		}

		if (path.stroke && path.weight !== 0) {
			this.ctx.globalAlpha = path.opacity;

			this.ctx.lineWidth = path.weight;
			this.ctx.strokeStyle = path.color;
			this.ctx.lineCap = path.lineCap;
			this.ctx.lineJoin = path.lineJoin;
			this.ctx.stroke();
		}

	}

	bringToFront(path: CPath) {
		// TODO: Implement this.
	}

	bringToBack(path: CPath) {
		// TODO: Implement this.
	}

	getMap(): any {
		return this.map;
	}
}