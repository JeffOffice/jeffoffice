# Segurança: até onde um arquivo pode acessar
<!-- keywords: central de confiabilidade, segurança, macros, arquivos vinculados, links, imagens vinculadas, mídia vinculada, internet, baixado, somente leitura, protegido, links externos, programas, seguro -->

Um arquivo pode apontar para outras coisas: outros arquivos, páginas da web, vídeos e até programas. A página **Segurança** é onde você decide o que o JeffCrunch faz com isso. Clique em **Arquivo > Opções** e, em seguida, na guia **Segurança**. Clique em **OK** para manter suas alterações ou em **Cancelar** para descartá-las.

Estas opções valem para todos os aplicativos do JeffOffice: se você alterar uma em um aplicativo, ela muda em todos. Uma linha para algo que um aplicativo não possui diz **Não se aplica** e fica em cinza.

## Macros

Macros são pequenos programas guardados dentro de um arquivo. O JeffCrunch os mantém no arquivo, para que continuem lá ao salvar, e **nunca os executa**. Não há nenhuma configuração que as execute.

## Arquivos vinculados e vínculos de pastas de trabalho

Trata-se das outras pastas de trabalho de onde uma pasta de trabalho obtém números, chamadas **vínculos**. A escolha define o que o JeffCrunch faz ao abrir uma pasta de trabalho que tenha alguns deles.

- **Perguntar antes** é a opção habitual. O JeffCrunch pergunta se deve atualizar os números das outras pastas de trabalho.
- **Atualizar** atualiza os números sem perguntar.
- **Nunca atualizar** mantém os números que o arquivo armazenou.

Uma escolha feita para uma pasta de trabalho em **Dados > Editar Links** (**Aviso de inicialização**) prevalece para essa pasta de trabalho. Uma pasta de trabalho marcada como "nunca" fica sempre intocada. Uma pasta de trabalho em outro computador nunca é lida ao abrir um arquivo, qualquer que seja a escolha. Você ainda pode usar **Atualizar Valores** em **Editar Links** sempre que quiser.

## Vídeos e sons vinculados

Isto não se aplica no JeffCrunch, pois ele não possui vídeos nem sons vinculados para reproduzir. A linha fica em cinza.

## Arquivos da internet ou de outros computadores

Um arquivo que você baixou, ou que está em outro computador da sua rede, pode não ser seguro. Esta opção define como o JeffCrunch o abre.

- **Abrir primeiro como somente leitura** é a opção habitual. O arquivo abre para que você possa lê-lo, mas não pode alterá-lo. Uma barra sob o arquivo informa isso. Clique em **Habilitar Edição** na barra para alterar somente esse arquivo (até você fechar o programa). Ao salvá-lo depois, a marca de download é removida do arquivo. Ou use **Salvar Como** para criar uma cópia, ou altere esta configuração para a opção seguinte.
- **Abrir normalmente** abre todos os arquivos da mesma forma.

Um arquivo aberto com **Abrir como Cópia**, **Abrir e Reparar** ou **Abrir como Somente Leitura** na tela **Arquivo** é aberto desse modo, qualquer que seja a configuração.

## Links que iniciam um programa

Um link que iniciaria um programa ou um script (um arquivo com extensão .exe, .bat, .sh e similares) é **sempre recusado**. O JeffCrunch avisa você, e nada é executado. Um link para um arquivo em outro computador nunca é seguido sem que você seja consultado antes. Nenhuma configuração altera isso.

## Veja também

- [Alterar as configurações do JeffCrunch](options)
- [Notas e links](notes-links)
