use slint::ComponentHandle;

slint::include_modules!();

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let ui = AppWindow::new()?;

    let weak = ui.as_weak();
    ui.window().on_close_requested(move || {
        if let Some(w) = weak.upgrade() {
            let _ = w.hide();
        }
        slint::CloseRequestResponse::HideWindow
    });

    ui.run()?;
    Ok(())
}
