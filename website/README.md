# website

The BitBack product page: one static page (`index.html` + `styles.css`), no framework and no JavaScript. Open
`index.html` in a browser to see it. Everything it needs is in this folder - also its font, so no visitor data goes
to Google Fonts (German courts have fined sites for that).

**Not ready to publish:**
- the price is `[PRICE]`, and the Buy buttons don't go anywhere - the payment provider isn't chosen yet (the 2014
  site sold through ClickBank)
- Imprint, Privacy policy and Licence terms are links to nowhere - the first two are required by law in Germany
- the Facebook and YouTube links are the 2014 site's accounts - check they are still yours
- `fonts/` needs Lato's licence text (`OFL.txt`, SIL Open Font License 1.1) beside the font files
- the screenshot (`img/app-screenshot.png`) shows a test disk image and the developer's own drives

## Files

- `index.html`, `styles.css` - the page
- `img/app-screenshot.png` - the real app, scanning a disk image (taken from version 2.7.0)
- `img/logo.png`, `img/logo-white.png` - the BitBack logo, for light and dark backgrounds
- `img/feature-*.png` - the feature icons, `img/product-box.png` - the product box, `img/social/` - icons
- `img/badge-lifetime-licence.png` - not used yet: only if the licence really is for life (one payment, free updates)
- `favicon.ico`, `fonts/Lato-Light.woff` and `.ttf`
- `.htaccess` - gzip compression on an Apache server
- `content-2014.md` - the 2014 site's text, with notes on what changed - the page's text was written from it
- `legacy-2014/` - the old August 2014 draft of the page (Bootstrap, jQuery), kept for reference. Not to be
  published: placeholder reviews, a Terms of Use template from a brokerage, outdated claims.

## Third-party code and fonts

| What | Licence |
|---|---|
| Lato (`fonts/`), by Łukasz Dziedzic | SIL Open Font License 1.1 - its licence text has to go with the font files |

`legacy-2014/` also bundles Bootstrap 3.1.1 / 3.2.0, Animate.css, jQuery 1.7.2, WOW.js 0.1.12, Glide.js 1.0.65
(all MIT) and jParallax by stephband (licence not stated in its files).
