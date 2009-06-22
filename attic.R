

## old R functions removed as part of cleanup:


## FIXME: make these layout widgets, with a thin child for tick marks

qxaxis <-
    function(xlim, tick.number = 5,
             at = pretty(xlim, tick.number),
             labels = as.character(at),
             font = qv.font(),
             side = c("bottom", "top"),
             ## y = switch(side, bottom = 1, top = 0),
             tck = 1,
             rot = 0,
             hadj = 0.75,
             vadj = 0.25, ##switch(side, bottom = 1, top = -0.5),
             minheight = 0, minwidth = 0)
{
    force(labels)
    id <- at >= xlim[1] & at <= xlim[2]
    at <- at[id]
    labels <- labels[id]
    side <- match.arg(side)
    labsize <- lapply(qvStrSize(labels, font = font), max)
    if (missing(minheight))
    {
        minheight <- ## FIXME: need more work for other rot 
            if (rot == 90) labsize$width
            else 1.2 * labsize$height
##         str(list(labels = labels,
##                  labsize = labsize,
##                  rot = rot))
    }
    labelRenderInfo <- 
    {
        qtpaint:::resetPanelInfo()
        qv.setScale(xlim, c(0, 1))
        qv.setPar(col = "black")
        qv.setFont(font = font)
        qv.panel.text(at, 0.5, labels,
                      recycle = TRUE,
                      rot = rot,
                      hadj = hadj, vadj = vadj)
        qtpaint:::getPanelInfo()
    }
    tickRenderInfo <- 
    {
        qtpaint:::resetPanelInfo()
        qv.setScale(xlim, c(0, 1))
        qv.setPar(col = "black")
        qv.panel.abline(v = at)
        qtpaint:::getPanelInfo()
    }
    labelWidget <- 
        qvBasicWidget(labelRenderInfo,
                      xexpand = TRUE,
                      yexpand = FALSE,
                      minheight = minheight,
                      minwidth = minwidth)
    if (tck == 0) return(labelWidget)
    tickWidget <-
        qvBasicWidget(tickRenderInfo,
                      xexpand = TRUE,
                      yexpand = FALSE,
                      minheight = tck * 8,
                      minwidth = minwidth)
    ans <- qvBasicWidget(xexpand = TRUE, yexpand = FALSE)
    ans[2, 2] <- labelWidget
    ans[switch(side, bottom = 1, top = 3), 2] <- tickWidget
    ans
}


qyaxis <-
    function(ylim, tick.number = 5,
             at = pretty(ylim, tick.number),
             labels = as.character(at),
             font = qv.font(),
             side = c("left", "right"),
             x = switch(side, left = 1, right = 0),
             tck = 1,
             rot = 0,
             hadj = switch(side, left = 1, right = 0),
             vadj = 0.25,
             minheight = 0, minwidth = 0)
{
    force(labels)
    side <- match.arg(side)
    labels <- switch(side, # FIXME: temporary hack
                     left = paste(labels, "-"),
                     right = paste("-", labels))
    id <- at >= ylim[1] & at <= ylim[2]
    at <- at[id]
    labels <- labels[id]
    labsize <- lapply(qvStrSize(labels, font = font), max)
    if (missing(minwidth))
    {
        minwidth <- ## FIXME: need more work for other rot 
            if (rot == 90) labsize$height
            else labsize$width
    }
    renderInfo <- 
    {
        qtpaint:::resetPanelInfo()
        qv.setScale(c(0, 1), ylim)
        ## qv.panel.fill(col = "transparent", border = "black")
        ## qv.setPar(col = "grey")
        ## qv.panel.abline(h = at)
        qv.setPar(col = "black")
        qv.setFont(font = font)
        qv.panel.text(x, at, labels,
                      recycle = TRUE,
                      rot = rot,
                      hadj = hadj, vadj = vadj)
        qtpaint:::getPanelInfo()
    }
    qvBasicWidget(renderInfo,
                  xexpand = FALSE,
                  yexpand = TRUE,
                  minheight = minheight,
                  minwidth = minwidth)
}


## R wrappers to create Qt-level widgets.

qvBasicWidget <-
    function(render_info = NULL,
             x1 = 0, x2 = 1,
             y1 = 0, y2 = 1,
             xexpand = TRUE, yexpand = TRUE,
             minheight = 0L, minwidth = 0L,
             margin = 0L,
             parent = NULL, # ignored for now
             tag = NULL,
             canvas = "transparent",
             children = list())
{
    if (missing(render_info))
        render_info <- 
    {
        resetPanelInfo()
        qv.setPar(fill = canvas, col = canvas)
        qv.panel.rect(x = 0.5, y = 0.5,
                      width = 1, height = 1)
        getPanelInfo()
    }
    e <- 
        .Call("newBasicWidget",
              render_info,
              as.double(x1),
              as.double(x2),
              as.double(y1),
              as.double(y2),
              as.integer(xexpand),
              as.integer(yexpand),
              as.integer(minheight),
              as.integer(minwidth),
              as.integer(margin),
              tag,
              PACKAGE = "qtpaint")
    reg.finalizer(e, qvFinalizeBasicWidget, FALSE)
    ans <- list(extp = e, children = children)
    class(ans) <- "qwidget"
    ans
}



qvSetMinHeight <- function(x, h)
{
    .Call("setWidgetMinHeight", x$extp, as.integer(h),
          PACKAGE = "qtpaint")
    invisible()
}
qvSetMinWidth <- function(x, w)
{
    .Call("setWidgetMinWidth", x$extp, as.integer(w),
          PACKAGE = "qtpaint")
    invisible()
}
qvSetMinSize <- function(x, w, h)
{
    if (!missing(w)) qvSetMinWidth(x, w)
    if (!missing(h)) qvSetMinHeight(x, h)
}

qvSetHeight <- function(x, h)
{
    qvSetSize(x, w = NULL, h = h)
}
qvSetWidth <- function(x, w)
{
    qvSetSize(x, w = w, h = NULL)
}
qvSetSize <- function(x, w = NULL, h = NULL)
{
    .Call("setWidgetSize",
          x$extp,
          as.integer(w),
          as.integer(h),
          PACKAGE = "qtpaint")
    invisible()
}



qvShowWidget <- function(x)
{
    .Call("showWidget", x$extp, PACKAGE = "qtpaint")
    invisible()
}

print.qwidget <- function(x, ...)
{
    ##     .qvPrivateEnv$last.printed <- x
    ##     gc()
    qvShowWidget(x)
    invisible(x)
}


qvCloseWidget <- function(x) 
{
    if (inherits(x, "qwidget"))
        .Call("closeWidget", x$extp, PACKAGE = "qtpaint")
    else
        .Call("closeWidget", x, PACKAGE = "qtpaint")
    invisible()
}

qvUnparentWidget <- function(x) 
{
    if (inherits(x, "qwidget"))
        .Call("unparentWidget", x$extp, PACKAGE = "qtpaint")
    else
        .Call("unparentWidget", x, PACKAGE = "qtpaint")
    invisible(x)
}


qvFinalizeBasicWidget <- function(x)
{
    if (inherits(x, "qwidget")) 
    {
        ## cat("qvFinalizeBasicWidget called on qwidget\n")
        x$children <- list(NULL)
        invisible(.Call("finalizeBasicWidget", x$extp, PACKAGE = "qtpaint"))
    }
    else
    {
        ## cat("qvFinalizeBasicWidget called on external pointer: ")
        ## print(x)
        invisible(.Call("finalizeBasicWidget", x, PACKAGE = "qtpaint"))
    }
}

qvFinalizeWidget <- function(x) #, closeFirst = TRUE) 
{
    ## cat("qvFinalizeWidget called on ", class(x), "\n")
    ## if (closeFirst) qvCloseWidget(x)
    if (inherits(x, "qwidget")) 
    {
        x$children <- NULL
        invisible(.Call("finalizeWidget", x$extp, PACKAGE = "qtpaint"))
    }
    else
        invisible(.Call("finalizeWidget", x, PACKAGE = "qtpaint"))
}

qvFinalizeObject <- qvFinalizeWidget


qvAddWidget <-
    function(w, target,
             row, col, nrow = 1L, ncol = 1L,
             id = "",
             store = FALSE, warn.replace = FALSE)
{
    if (is.null(w)) return(invisible(target)) # w <- qvBasicWidget()
    .Call("addWidgetInLayout", w$extp, target$extp,
          as.integer(row - 1L),
          as.integer(col - 1L),
          as.integer(nrow),
          as.integer(ncol),
          PACKAGE = "qtpaint")
    if (store)
    {

        ## NOTE: this allows a child to be associated with a parent at
        ## the R level, and subsequently replaced.  However, other
        ## references to the same objects may exist, and they may be
        ## manipulated in other ways without updating the record here
        ## (e.g., a widget added here may subsequently be added to a
        ## different parent, changing its parent.  If a reference to
        ## the child is stored here, and is unparented, then that will
        ## have the confusing effect of unparenting the object from
        ## its current parent).  Upshot: use this facility with care.
        
        cname <- childName(row, col, nrow, ncol, id)
        if (!is.null(target$children[[cname]]))
        {
            if (warn.replace) warning("Replacing existing widget")
            qvUnparentWidget(target$children[[cname]])
        }
        target$children[[cname]] <- w
    }
    invisible(target)
}


"[.qwidget" <- function(x, i, j, ..., drop = FALSE)
{
    x$children[[childName(i, j, ...)]]
}



"[<-.qwidget" <-
    function (x, i, j, ..., value)
{
    qvAddWidget(value, x, row = i, col = j, ..., store = TRUE)
}



## a browser widget

qvBrowserWidget <-
    function(url = "http://www.r-project.org")
{
    e <- 
        .Call("newBrowserWidget",
              as.character(url[1]),
              PACKAGE = "qtpaint")
    reg.finalizer(e, qvFinalizeWidget, FALSE)
    ans <- list(extp = e, url = url)
    class(ans) <- "qwidget"
    ans
}

qvSetUrl <-
    function(x, url)
{
    .Call("setWidgetUrl", x$extp, as.character(url[1]),
          PACKAGE = "qtpaint")
    invisible()
}

qvRenderWidget <- function(x, view = FALSE)
{
    if (view)
        .Call("renderViewWidget",
              if (inherits(x, "qwidget")) x$extp else x,
              PACKAGE = "qtpaint")
    else .Call("renderWidget",
               if (inherits(x, "qwidget")) x$extp else x,
               NULL,
               PACKAGE = "qtpaint")
    invisible()
}

qvPixmap <- function(x)
{
    .Call("renderWidgetToPixmap",
          if (inherits(x, "qwidget")) x$extp else x,
          NULL,
          PACKAGE = "qtpaint")
    invisible()
}


qvSaveWidget <- function(x, file = "qvplot.pdf", type)
{
    stopifnot(length(file) == 1)
    extension <- tail(strsplit(basename(file), ".", fixed = TRUE)[[1]], 1)
    if (missing(type))
        type <-
            if (tolower(extension) %in% c("ps", "pdf")) "vector"
            else if (tolower(extension) %in% "svg") "svg"
            else "raster"
    switch(type,
           svg = .Call("renderWidgetToSVG",
                       if (inherits(x, "qwidget")) x$extp else x,
                       as.character(file), 
                       PACKAGE = "qtpaint"),
           raster = .Call("renderWidgetToPixmap",
                          if (inherits(x, "qwidget")) x$extp else x,
                          as.character(file), 
                          PACKAGE = "qtpaint"),
           vector = .Call("renderWidget",
                          if (inherits(x, "qwidget")) x$extp else x,
                          as.character(file),
                          PACKAGE = "qtpaint"))
    invisible()
}



qv.panel.densityplot <-
    function(x, y, col = "blue", alpha = 1)
{
    qv.setPar(col = col, alpha = alpha)
    qv.panel.lines(x, y)
}

qdensity <-
    function(x, col = "blue",
             panel = qv.panel.densityplot,
             xlim, ylim, ...)
{
    renderInfo <- 
        if (sum(!is.na(x)) > 1)
        {
            h <- density(x = x, ...)
            xr <- if (missing(xlim)) extendrange(h$x) else xlim
            yr <- if (missing(ylim)) extendrange(h$y) else ylim
            id <- h$x > min(xr) & h$x < max(xr)
            qtpaint:::resetPanelInfo()
            qv.setScale(xr, yr)
            qv.panel.lines(x = xr[c(1, 2, 2, 1, 1)],
                           y = yr[c(1, 1, 2, 2, 1)])
            panel(h$x, h$y, ...)
            qtpaint:::getPanelInfo()
        }
        else list()
    qvBasicWidget(renderInfo)
}


## basic high-level functions

qv.densityplot <-
    function(x, ...,
             xlab = deparse(substitute(x)),
             ylab = "Density",
             main = "")
{
    clist <- list()
    dens <- density(x, ...)
    xlim <- extendrange(dens$x)
    ylim <- extendrange(dens$y)
    clist$d <-
        with(dens,
             qscatter(x, y,
                      xlim = xlim, ylim = ylim,
                      type = "l"))
    clist$p <- qscatter(x, (-0.5 * ylim[1]) * (2 * runif(length(x)) - 1),
                      xlim = xlim, ylim = ylim)
    clist$xax <- qxaxis(xlim, minheight = 50)
    clist$yax <- qyaxis(ylim, minwidth = 20)
    clist$labx <- qlabel(xlab, minheight = 20, col = "red")
    clist$laby <- qlabel(ylab, horizontal = FALSE, minwidth = 20, col = "red")
    clist$labmain <- qlabel(main, minheight = 20, col = "red")
    tmp <- qvBasicWidget(tag = clist)
    qvAddWidget(clist$p, tmp, 2, 3)
    qvAddWidget(clist$d, tmp, 2, 3)
    qvAddWidget(clist$xax, tmp, 3, 3)
    qvAddWidget(clist$yax, tmp, 2, 2)
    qvAddWidget(clist$laby, tmp, 2, 1)
    qvAddWidget(clist$labx, tmp, 4, 3)
    qvAddWidget(clist$labmain, tmp, 1, 1, 1, 3)
    tmp
}


qv.xyplot <-
    function(x, y = NULL,
             xlim = NULL, ylim = NULL,
             xlab = NULL, ylab = NULL,
             main = "",
             type = c("g", "p"),
             ...)
{
    xlabel <- if (!missing(x)) 
        deparse(substitute(x))
    ylabel <- if (!missing(y)) 
        deparse(substitute(y))
    xy <- xy.coords(x, y, xlabel, ylabel)
    xlab <- if (is.null(xlab)) xy$xlab else xlab
    ylab <- if (is.null(ylab)) xy$ylab else ylab
    xlim <- if (is.null(xlim)) extendrange(xy$x[is.finite(xy$x)]) else xlim
    ylim <- if (is.null(ylim)) extendrange(xy$y[is.finite(xy$y)]) else ylim
    p <-
        with(xy,
             qscatter(x, y,
                      xlim = xlim, ylim = ylim,
                      type = type, ...))
    xax <- qxaxis(xlim, minheight = 50)
    yax <- qyaxis(ylim, minwidth = 70)
    labx <- qlabel(xlab, minheight = 20, col = "red")
    laby <- qlabel(ylab, horizontal = FALSE, minwidth = 20, col = "red")
    labmain <- qlabel(main, minheight = 20, col = "red")
    tmp <- qvBasicWidget()
    ## tag = list(p, xax, yax, labx, laby, labmain)
    tmp <- qvAddWidget(p, tmp, 2, 3)
    tmp <- qvAddWidget(xax, tmp, 3, 3)
    tmp <- qvAddWidget(yax, tmp, 2, 2)
    tmp <- qvAddWidget(laby, tmp, 2, 1)
    tmp <- qvAddWidget(labx, tmp, 4, 3)
    tmp <- qvAddWidget(labmain, tmp, 1, 1, 1, 3)
    tmp
}


do.breaks <- 
    function (endpoints, nint)
{
    if (length(endpoints) != 2)
        stop("error")
    endpoints[1] + diff(endpoints) * 0:nint/nint
}

qhist.constructor <- 
    function (x, breaks, include.lowest = TRUE, right = TRUE, ...)
{
    if (is.numeric(breaks) && length(breaks) > 1)
        hist(as.numeric(x), breaks = breaks, plot = FALSE,
             include.lowest = include.lowest,
             right = right)
    else hist(as.numeric(x), breaks = breaks, right = right,
              plot = FALSE)
}

qv.panel.histogram <-
    function(xleft, ybottom,
             width, height,
             fill = "cyan", ...)
{
    qv.setPar(fill = fill, ...)
    qv.panel.rect(xleft = xleft, 
                  ybottom = ybottom,
                  xright = xleft + width,
                  ytop = ybottom + height)
}

qhistogram <-
    function(x,
             breaks = NULL,
             equal.widths = TRUE,
             type = "density",
             nint = round(log2(length(x)) + 1),
             panel = qv.panel.histogram,
             xlim, ylim, ...)
{
    renderInfo <- 
        if (any(is.finite(x)))
        {
            if (is.null(breaks))
            {
                breaks <-
                    if (is.factor(x)) seq_len(1 + nlevels(x)) - 0.5
                    else if (equal.widths) do.breaks(range(x, finite = TRUE), nint)
                    else quantile(x, 0:nint/nint, na.rm = TRUE)
            }
            h <- qhist.constructor(x, breaks = breaks, ...)
            y <-
                switch(type,
                       count = h$counts,
                       percent = 100 * h$counts/length(x),
                       density = h$intensities)
            breaks <- h$breaks
            nb <- length(breaks)
            if (length(y) != nb-1)
                warning("problem with 'hist' computations")
            xr <-
                if (missing(xlim))
                    extendrange(range(x, h$breaks, na.rm = TRUE))
                else xlim
            yr <-
                if (missing(ylim))
                    extendrange(range(0, y, finite = TRUE))
                else ylim
            qtpaint:::resetPanelInfo()
            qv.setScale(xr, yr)
            qv.panel.lines(x = xr[c(1, 2, 2, 1, 1)],
                           y = yr[c(1, 1, 2, 2, 1)])
            if (nb > 1)
                panel(x = breaks[-nb],
                      y = 0,
                      width = diff(breaks),
                      height = y,
                      ...)
            qtpaint:::getPanelInfo()
        }
        else list()
    ## str(renderInfo)
    qvBasicWidget(renderInfo)
}



qlabel <-
    function(label, horizontal = TRUE, font = qv.font(),
             rot = if (horizontal) 0 else 90,
             bg = "transparent", border = "transparent", # ignored
             col = "black",
             hadj = 0.5, vadj = 0.25,
             minheight = 0, minwidth = 0,
             xexpand = horizontal,
             yexpand = !horizontal,
             ...)
{
    if (length(label) > 1) {
        warning("using first element of 'label'")
        label <- label[1]
    }
    labsize <- qvStrSize(label, font = font)
    if (missing(minheight))
    {
        minheight <-
            if (horizontal)
            {
                ## FIXME: need more work for other rot, but would
                ## simplify calculations.  Only need to work with bounding box
                if (rot == 90) labsize$width
                else labsize$height
            }
            else 
            {
                if (rot == 90) labsize$height
                else labsize$width
            }
    }
    if (missing(minwidth))
    {
        minwidth <-
            if (!horizontal)
            {
                ## FIXME: need more work for other rot 
                if (rot == 90) labsize$height
                else labsize$width
            }
            else 
            {
                if (rot == 90) labsize$width
                else labsize$height
            }
    }
    renderInfo <- 
    {
        qtpaint:::resetPanelInfo()
        qv.setScale(c(0, 1), c(0, 1))
        qv.setFont(font = font)
        qv.setPar(fill = bg, col = border)
        qv.panel.rect(x = 0.5, y = 0.5,
                      width = 1, height = 1)
        qv.setPar(col = col, ...)
        qv.panel.text(0.5, 0.5, label,
                      rot = rot, hadj = hadj, vadj = vadj)
        qtpaint:::getPanelInfo()
    }
    qvBasicWidget(renderInfo,
                  xexpand = xexpand,
                  yexpand = yexpand,
                  minheight = minheight,
                  minwidth = minwidth)
}


qv.panel.scatter <-
    function(x, y, col = "blue", fill = "blue", alpha = 0.5,
             pch = NULL,
             type = "p", ...)
{
    qv.setPar(col = col, fill = fill, alpha = alpha, ...)
    if ("g" %in% type) qv.panel.grid()
    if ("p" %in% type)
        if (is.null(pch)) qv.panel.points(x, y)
        else qv.panel.text(x, y, labels = rep(pch, length = length(x)))
    if ("l" %in% type) qv.panel.lines(x, y)
}

qscatter <-
    function(x, y = NULL, ...,
             panel = qv.panel.scatter,
             xlim, ylim)
{
    xy <- xy.coords(x, y, recycle = TRUE)
    rng <- lapply(xy[c("x", "y")], extendrange)
    xr <- if (missing(xlim)) rng[["x"]] else xlim
    yr <- if (missing(ylim)) rng[["y"]] else ylim
    renderInfo <- 
    {
        qtpaint:::resetPanelInfo()
        qv.setScale(xr, yr)
        qv.panel.lines(x = xr[c(1, 2, 2, 1, 1)],
                       y = yr[c(1, 1, 2, 2, 1)])
        panel(xy$x, xy$y, ...)
        qtpaint:::getPanelInfo()
    }
    qvBasicWidget(renderInfo)
}





qvGetRenderInfo <- function(x) 
{
  .Call("getWidgetRenderInfo", x$extp, PACKAGE = "qtpaint")
  invisible()
}


childName <- function(row, col, nrow = 1L, ncol = 1L, id = "")
{
  if (max(length(row), length(col), length(nrow), length(ncol), length(id)) != 1)
    stop("")
  sprintf("%g,%g,%g,%g,%s",
          as.integer(row), as.integer(col),
          as.integer(nrow), as.integer(ncol),
          as.character(id))
}




qvStrSize <- function(x, font = qv.font(), rot = 0) # 'rot' unused for now
{
  ## 'x': character vector
  ## FIXME: taking shortcut - preallocating result here; should create inside .Call() 
  widths <- integer(length(x))
  heights <- integer(length(x))
  .Call("stringBoundingBox",
        x, font, widths, heights,
        PACKAGE = "qtpaint")
  list(widths = widths, heights = heights)
}



## functions that look like standard R graphics primitives or lattice
## panel functions, but actually end up building up a list of Qt
## primitive calls


.PanelEnv <- new.env(parent = emptyenv())


resetPanelInfo <- function(init.length = 50)
{
  ## initialize to be moderatly long, to save a bit of copying (I hope)
  .PanelEnv[["info"]] <-
    vector(mode = "list", length = init.length)
  .PanelEnv[["index"]] <- 0
  .PanelEnv[["xlim"]] <- c(0, 1)
  .PanelEnv[["ylim"]] <- c(0, 1)
  .PanelEnv[["font"]] <- qv.font()
}


addPanelInfo <- function(name, args)
{
  .PanelEnv[["index"]] <- .PanelEnv[["index"]] + 1L
  .PanelEnv[["info"]][[ .PanelEnv[["index"]] ]] <-
    list(name, args)
}


getPanelInfo <- function()
{
  head(.PanelEnv[["info"]],
       .PanelEnv[["index"]])
}

qv.getScale <- function()
{
  list(xlim = .PanelEnv[["xlim"]],
       ylim = .PanelEnv[["ylim"]])
}


qv.setScale <-
  function(xlim = c(NA_real_, NA_real_),
           ylim = c(NA_real_, NA_real_))
{
  xlim <- rep(as.numeric(xlim), length = 2)
  ylim <- rep(as.numeric(ylim), length = 2)
  .PanelEnv[["xlim"]] <- xlim
  .PanelEnv[["ylim"]] <- ylim
  if (!is.na(xlim[1])) addPanelInfo("setX1", list(xlim[1]))
  if (!is.na(xlim[2])) addPanelInfo("setX2", list(xlim[2]))
  if (!is.na(ylim[1])) addPanelInfo("setY1", list(ylim[1]))
  if (!is.na(ylim[2])) addPanelInfo("setY2", list(ylim[2]))
}


showContext <- function() {
  cat(head(sapply(lapply(sys.calls(), "[[", 1),
                  as.character), -1),
      sep = " -> ")
  cat("\n")
}

qv.par <-
  function(col = "black", fill = "transparent",
           lty = 1, lwd = 0, cex = 1, alpha = 1, ...)
{
  list(col = col, fill = fill,
       lty = lty, lwd = lwd,
       cex = cex, alpha = alpha)
}

qv.getPar <- function()
{
  ans <- .PanelEnv[["par"]]
  if (is.null(ans)) {
    ## qv.setPar(par = qv.par())
    ## .PanelEnv[["par"]]
    qv.par()
  }
  else ans
}

qv.setPar <-
  function(..., par = list(...))
{
  ## showContext()
  npar <- opar <- qv.getPar()
  nms <- names(par)
  if ("alpha" %in% nms)
    {
      if (length(par[["alpha"]]) != 1) warning("'alpha' must be scalar.")
      npar[["alpha"]] <- as.integer(255 * par[["alpha"]][1])
    }
  if ("col" %in% nms)
    {
      if (length(par[["col"]]) != 1) warning("'col' must be scalar.")
      npar[["col"]] <- par[["col"]][1]
      rgbmat <- col2rgb(npar[["col"]], alpha = TRUE)
      alpha <- if ("alpha" %in% nms) npar[["alpha"]] else rgbmat[4]
      addPanelInfo("setPlotColor",
                   list(rgbmat[1], rgbmat[2], rgbmat[3], alpha))
    }
  if ("fill" %in% nms)
    {
      if (length(par[["fill"]]) != 1) warning("'fill' must be scalar.")
      npar[["fill"]] <- par[["fill"]][1]
      rgbmat <- col2rgb(npar[["fill"]], alpha = TRUE)
      alpha <- if ("alpha" %in% nms) npar[["alpha"]] else rgbmat[4]
      addPanelInfo("setFillColor",
                   list(rgbmat[1], rgbmat[2], rgbmat[3], alpha))
    }
  .PanelEnv[["par"]] <- npar
}


qv.getFont <- function()
{
  .PanelEnv[["font"]]
}
qv.setFont <- function(..., font = qv.font(...))
{
  .PanelEnv[["font"]] <- font
  addPanelInfo("setFont", font)
}

qv.getAntialias <- function()
{
  ans <- .PanelEnv[["antialias"]]
  if (is.null(ans)) TRUE else ans
}
qv.setAntialias <- function(x = TRUE)
{
  .PanelEnv[["antialias"]] <- x
  addPanelInfo("setAntialias", list(as.integer(x)))
}

qv.panel.points <-
  function(x, y = NULL, col, fill, alpha, recycle = FALSE)
{
  ## if (!missing(col)) etc, use vector versions (not written yet)
  xy <- xy.coords(x, y, recycle = recycle)
  addPanelInfo("panelPoints", list(xy$x, xy$y))
}

qv.panel.lines <-
  function(x, y = NULL, col, fill, alpha, recycle = FALSE)
{
  ## if (!missing(col)) etc, use vector versions (not written yet)
  xy <- xy.coords(x, y, recycle = recycle)
  addPanelInfo("panelLines", list(xy$x, xy$y))
}

qv.panel.text <-
  function(x, y = NULL, labels = seq_along(x),
           rot = 0, hadj = 0.5, vadj = 0.5, 
           recycle = FALSE)
{
  labels <- as.character(labels)
  xy <- xy.coords(x, y, recycle = recycle)
  if (recycle) labels <- rep(labels, length = length(xy[["x"]]))
  stopifnot(length(labels) == length(xy[["x"]]))
  addPanelInfo("panelText",
               list(xy$x, xy$y, labels,
                    as.double(rot),
                    as.double(hadj),
                    as.double(vadj)))
}

qv.panel.polyline <-
  function(x, y = NULL, recycle = FALSE)
{
  xy <- xy.coords(x, y, recycle = recycle)
  addPanelInfo("panelPolyline", list(xy$x, xy$y))
}

qv.panel.polygon <-
  function(x, y = NULL, recycle = FALSE, fill = "transparent", col = "black")
{
  xy <- xy.coords(x, y, recycle = recycle)
  with(xy, {
    n <- length(x)
    w <- which(is.na(x) | is.na(y))
    id.starts <- c(1L, w+1L)
    id.ends <- c(w-1L, n)
    fill <- rep(fill, length = length(id.starts))
    col <- rep(col, length = length(id.starts))
    for (i in seq_along(id.starts))
      {
        id <- seq.int(from = id.starts[i],
                      to = id.ends[i], by = 1L)
        qv.setPar(fill = fill[i], col = col[i])
        addPanelInfo("panelPolygon", list(x[id], y[id]))
      }
  })
}

qv.panel.rect <-
  function(xleft, ybottom, xright, ytop,
           x = (xleft + xright)/2, y = (ybottom + ytop)/2,
           width = xright - xleft, height = ytop - ybottom,
           recycle = TRUE)
{
  xy <- xy.coords(x, y, recycle = recycle)
  width <- rep(width, length = length(xy$x))
  height <- rep(height, length = length(xy$y))
  addPanelInfo("panelRect", list(xy$x, xy$y, width, height))
}


qv.panel.loess <-
  function(x, y, span = 2/3, degree = 1,
           family = c("symmetric", "gaussian"),
           evaluation = 50,
           horizontal = FALSE,
           ...)
{
  x <- as.numeric(x)
  y <- as.numeric(y)
  ok <- is.finite(x) & is.finite(y)
  if (sum(ok) < 1) return()
  if (horizontal)
    {
      smooth <-
        loess.smooth(y[ok], x[ok], span = span, family = family,
                     degree = degree, evaluation = evaluation)
      qv.panel.lines(x = smooth$y, y = smooth$x)
    }
  else
    {
      smooth <-
        loess.smooth(x[ok], y[ok], span = span, family = family,
                     degree = degree, evaluation = evaluation)
      qv.panel.lines(x = smooth$x, y = smooth$y)
    }
}


qv.panel.segments <- function(x0, y0, x1, y1)
{
  xy <- xy.coords(x0, y0, recycle = TRUE)
  x1 <- rep(x1, length = length(xy$x))
  y1 <- rep(y1, length = length(xy$y))
  addPanelInfo("panelSegments", list(xy$x, xy$y, x1, y1))
}

qv.panel.abline <-
  function(a = NULL, b = NULL,
           h = NULL, v = NULL,
           reg = NULL, coef = NULL)
{
  if (!is.null(h) || !is.null(v))
    {
      h <- unique(h)
      v <- unique(v)
      nh <- length(h)
      nv <- length(v)
      cl <- qv.getScale()
      x0 <- c(numeric(0), rep(cl$xlim[1], nh), v)
      x1 <- c(numeric(0), rep(cl$xlim[2], nh), v)
      y0 <- c(numeric(0), h, rep(cl$ylim[1], nv))
      y1 <- c(numeric(0), h, rep(cl$ylim[2], nv))
      qv.panel.segments(x0, y0, x1, y1)
    }
  if (!is.null(reg)) 
    {
      if (is.null(coef)) warning("'coef' overridden by 'reg'")
      coef <- coef(reg)
    }
  if (!is.null(coef)) 
    {
      if (!(is.null(a) && is.null(b))) warning("'a' and 'b' overridden by 'coef'")
      a <- coef[1]
      b <- coef[2]
    }
  if (!is.null(a))
    {
      if (is.null(b)) 
        {
          if (length(a) == 2)
            {
              b <- a[2]
              a <- a[1]
            }
          else stop("'a' must have length 2 if 'b' is NULL")
        }
      if (any(!is.finite(b))) stop("all elements of 'b' must be finite; use 'v' instead.")
      fabline <- function(x) { a + b * x }
      fbaline <- function(y) { (y - a) / b } ## b shouldn't be 0 then
      cl <- qv.getScale()
      y0 <- fabline(cl$xlim[1])
      y1 <- fabline(cl$xlim[2])
      if (any(c(y0,y1) < cl$ylim[1] |
              c(y0,y1) > cl$ylim[2])) warning("FIXME: potential clipping issues")
      if (FALSE) ## check b != 0, and what about lines completely outside?
        {
          x0 <- fbaline(cl$ylim[1])
          x1 <- fbaline(cl$ylim[2])
          ## do something with these
        }
      ## old.clip <- qv.setClipping(cl)
      qv.panel.segments(cl$xlim[1], y0, cl$xlim[2], y1)
      ## qv.setClipping(old.clip)
    }
}

qv.panel.lmline <- function(x, y, ...)
{
  if (length(x) > 1)
    {
      fm <- lm(as.numeric(y) ~ as.numeric(x))
      qv.panel.abline(coef = coef(fm), ...)
    }
}




qv.panel.grid <-
  function(h = 3, v = 3)
{
  h <- h[1]; v <- v[1]
  cl <- qv.getScale()
  qv.setScale(c(0,1), c(0,1))
  if (h > 0) qv.panel.abline(h = ppoints(h, a = 0))
  if (v > 0) qv.panel.abline(v = ppoints(v, a = 0))
  if (h < 0) qv.panel.abline(h = pretty(cl$ylim, n = -h))
  if (v < 0) qv.panel.abline(v = pretty(cl$xlim, n = -h))
  qv.setScale(cl$xlim, cl$ylim)
}


qv.panel.fill <-
  function(col = "grey", border = "black")
{
  qv.setPar(fill = col, col = border)
  cl <- qv.getScale()
  qv.panel.rect(cl$xlim[1], cl$ylim[1],
                cl$xlim[2], cl$ylim[2])
}




### others:

## qv.panel.fill
## qv.panel.grid
## qv.panel.abline
## qv.panel.loess
## qv.panel.segments
## qv.panel.
## qv.panel.
## qv.panel.




qvTransformView <-
  function(x,
           scale = 1, xscale = scale, yscale = scale,
           rotate = 0, shear = NULL,
           translate = c(0, 0))
{
  .Call("setViewTransform",
        if (inherits(x, "qwidget")) x$extp else x,
        as.double(xscale),
        as.double(yscale),
        as.double(rotate),
        as.double(rep(translate, length.out = 2)),
        PACKAGE = "qtpaint")
  invisible()
}


## FIXME: do all recycling in Qt code rather than here (why not?)

qvScene.points <- function(s, x, y, radius = 1)
{
  xy <- xy.coords(x, y, recycle = TRUE)
  invisible(.Call("scene_addPoints", s, as.double(xy$x), as.double(xy$y), as.double(radius), PACKAGE = "qtpaint"))
}

qvScene.lines <- function(s, x, y, lwd = 0)
{
  xy <- xy.coords(x, y, recycle = TRUE)
  invisible(.Call("scene_addLines", s, as.double(xy$x), as.double(xy$y), as.double(lwd), PACKAGE = "qtpaint"))
}

qvScene.segments <- function(s, x1, y1, x2, y2, lwd = 0)
{
  n <- max(length(x1), length(x2), length(y1), length(y2))
  invisible(.Call("scene_addSegments", s,
                  rep(as.double(x1), length.out = n),
                  rep(as.double(y1), length.out = n),
                  rep(as.double(x2), length.out = n),
                  rep(as.double(y2), length.out = n),
                  as.double(lwd),
                  PACKAGE = "qtpaint"))
}

qvScene.rect <- function(s, x, y, w = 1, h = 1)
{
  xy <- xy.coords(x, y, recycle = TRUE)
  invisible(.Call("scene_addRect", s, as.double(xy$x), as.double(xy$y), as.double(w), as.double(h), PACKAGE = "qtpaint"))
}


qvScene.text <- function(s, x, y, labels, html = FALSE)
{
  xy <- xy.coords(x, y, recycle = TRUE)
  invisible(.Call("scene_addText", s, as.double(xy$x), as.double(xy$y), as.character(labels), as.logical(html), PACKAGE = "qtpaint"))
}






qvView.setdrag <-
  function(v, mode = c("none", "scroll", "select"))
{
  mode <- match.arg(mode)
  imode <- switch(mode, scroll = 1L, select = 2L, 0L)
  invisible(.Call("view_setDragMode",
                  if (inherits(v, "qwidget")) v$extp else v,
                  as.integer(imode),
                  PACKAGE = "qtpaint"))
}


qvView.setantialias <-
  function(v, mode = TRUE)
{
  invisible(.Call("view_setAntialias",
                  if (inherits(v, "qwidget")) v$extp else v,
                  as.logical(mode),
                  PACKAGE = "qtpaint"))
}
