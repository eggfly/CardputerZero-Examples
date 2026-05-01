use slint::ComponentHandle;

slint::include_modules!();

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let ui = AppWindow::new()?;

    // Quit on Esc / Q via the FocusScope callback declared in appwindow.slint.
    ui.on_quit(|| {
        slint::quit_event_loop().ok();
    });

    // Fallback: also quit when the window manager requests close.
    let weak = ui.as_weak();
    ui.window().on_close_requested(move || {
        if let Some(w) = weak.upgrade() {
            let _ = w.hide();
        }
        slint::quit_event_loop().ok();
        slint::CloseRequestResponse::HideWindow
    });

    ui.run()?;
    Ok(())
}
