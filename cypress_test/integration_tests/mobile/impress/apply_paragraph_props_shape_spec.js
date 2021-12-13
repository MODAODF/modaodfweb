/* global describe it cy beforeEach require afterEach*/

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var impressMobileHelper = require('./impress_mobile_helper');

describe('Apply paragraph properties on selected shape.', function() {
	var testFileName = 'apply_paragraph_props_shape.odp';

	beforeEach(function() {
		helper.beforeAll(testFileName, 'impress');

		mobileHelper.enableEditingMobile();

		impressMobileHelper.selectTextShapeInTheCenter();
	});

	afterEach(function() {
		helper.afterAll(testFileName);
	});

	function triggerNewSVG() {
		mobileHelper.closeMobileWizard();
		impressMobileHelper.triggerNewSVGForShapeInTheCenter();
	}

	function openParagraphPropertiesPanel() {
		mobileHelper.openMobileWizard();

		helper.clickOnIdle('#ParaPropertyPanel');

		cy.get('.unoParaLeftToRight')
			.should('be.visible');
	}

	function openListsPropertiesPanel() {
		mobileHelper.openMobileWizard();

		helper.clickOnIdle('#ListsPropertyPanel');

		cy.get('.unoDefaultBullet')
			.should('be.visible');
	}

	// FIXME temporarily disabled, does not work with CanvasTileLayer
	it.skip('Apply left/right alignment on text shape.', function() {
		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');

		// Set right alignment first
		openParagraphPropertiesPanel();

		helper.clickOnIdle('.unoRightPara');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '23586');

		// Set left alignment
		openParagraphPropertiesPanel();

		helper.clickOnIdle('.unoLeftPara');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');
	});

	// FIXME temporarily disabled, does not work with CanvasTileLayer
	it.skip('Apply center alignment on text shape.', function() {
		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');

		openParagraphPropertiesPanel();

		helper.clickOnIdle('.unoCenterPara');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '12493');
	});

	// FIXME temporarily disabled, does not work with CanvasTileLayer
	it.skip('Apply justified alignment on text shape.', function() {
		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');

		// Set right alignment first
		openParagraphPropertiesPanel();

		helper.clickOnIdle('.unoRightPara');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '23586');

		// Then set justified alignment
		openParagraphPropertiesPanel();

		helper.clickOnIdle('.unoJustifyPara');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');
	});

	// FIXME temporarily disabled, does not work with CanvasTileLayer
	it.skip('Set top/bottom alignment on text shape.', function() {
		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '4834');

		// Set bottom alignment first
		openParagraphPropertiesPanel();

		helper.clickOnIdle('#CellVertBottom');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '10811');

		// Then set top alignment
		openParagraphPropertiesPanel();

		helper.clickOnIdle('.unoCellVertTop');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '4834');
	});

	// FIXME temporarily disabled, does not work with CanvasTileLayer
	it.skip('Apply center vertical alignment on text shape.', function() {
		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '4834');

		openParagraphPropertiesPanel();

		helper.clickOnIdle('.unoCellVertCenter');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '7823');
	});

	// FIXME temporarily disabled, does not work with CanvasTileLayer
	it.skip('Apply default bulleting on text shape.', function() {
		// We have no bulleting by default
		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .BulletChars')
			.should('not.exist');

		openListsPropertiesPanel();

		helper.clickOnIdle('#ListsPropertyPanel .unoDefaultBullet');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .BulletChars')
			.should('exist');
	});

	// FIXME temporarily disabled, does not work with CanvasTileLayer
	it.skip('Apply default numbering on text shape.', function() {
		// We have no bulleting by default
		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextShape tspan')
			.should('not.have.attr', 'ooo:numbering-type');

		openListsPropertiesPanel();

		helper.clickOnIdle('#ListsPropertyPanel .unoDefaultNumbering');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextShape tspan')
			.should('have.attr', 'ooo:numbering-type', 'number-style');
	});

	// FIXME temporarily disabled, does not work with CanvasTileLayer
	it.skip('Apply spacing above on text shape.', function() {
		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '6600');

		openParagraphPropertiesPanel();

		cy.get('#aboveparaspacing input')
			.clear()
			.type('2{enter}');

		cy.get('#aboveparaspacing input')
			.should('have.attr', 'value', '2');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '11180');
	});

	// FIXME temporarily disabled, does not work with CanvasTileLayer
	it.skip('Apply spacing below on text shape.', function() {
		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '6600');

		openParagraphPropertiesPanel();

		cy.get('#belowparaspacing input')
			.clear()
			.type('2{enter}');

		cy.get('#belowparaspacing input')
			.should('have.attr', 'value', '2');

		triggerNewSVG();

		cy.get('.leaflet-pane.leaflet-overlay-pane g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '11180');
	});
});
