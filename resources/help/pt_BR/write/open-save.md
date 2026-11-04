# Abrir, salvar, imprimir e exportar
<!-- keywords: abrir, salvar, salvar como, docx, doc, dot, odt, opendocument, rtf, arquivos antigos, modelo, dotx, recentes, fixar, novo, fechar, imprimir, pdf, exportar, recuperar, recuperação automática, não salvo, travamento, tela arquivo, formatos de arquivo, salvar uma cópia, somente leitura, abrir como cópia, nova janela, cópia de backup, meus modelos, salvar como modelo, ott, reparar, abrir e reparar, danificado -->

Clique em **Arquivo**, no canto superior esquerdo, para ver a tela Arquivo. Ela tem as páginas Novo, Abrir, Imprimir e Exportar, além dos comandos Salvar e Salvar Como. Pressione **Esc** para voltar ao seu documento.

## Iniciar um novo documento

Pressione **Ctrl+N**, ou clique em **Arquivo > Novo** e depois em **Documento em Branco**. Um novo documento abre em uma janela própria. Se a janela em que você está estiver vazia e nunca tiver sido salva, o novo documento toma o lugar dela.

## Abrir um documento

Pressione **Ctrl+O**, ou clique em **Arquivo > Abrir**. Clique em **Procurar** para escolher um arquivo. O JeffWrite abre estes tipos de arquivo:

- **.docx** é um documento.
- **.dotx** é um modelo. Um modelo abre como um novo documento feito a partir dele, de modo que o modelo continua como está.
- **.docm** e **.dotm** também abrem. O JeffWrite não mantém as macros deles. Ele avisa você na primeira vez que salvar esse documento e salva um arquivo .docx.
- **.doc** e **.dot** são os tipos mais antigos de documento e modelo. O JeffWrite os lê e salva o que você fizer com eles como um novo arquivo .docx.
- **.odt**, **.ott** e **.fodt** são arquivos OpenDocument. O JeffWrite lê os três. Ele salva arquivos .odt e salva o que você fizer com um arquivo .ott ou .fodt como um novo arquivo .odt. Veja [Abrir e salvar arquivos OpenDocument](open-odt).
- **.rtf** é o Rich Text Format. O JeffWrite o lê e salva o que você fizer com ele como um novo arquivo .docx. Veja [Abrir arquivos RTF](open-rtf).

A página **Abrir** também lista os documentos que você abriu ou salvou recentemente. Clique em um deles para abri-lo. Clique com o botão direito em um deles para fixá-lo no topo da lista ou removê-lo da lista.

Se um arquivo tiver partes que o JeffWrite ainda não consegue editar, como o layout de uma tabela ou um cabeçalho, ele as mantém. Veja [Boas-vindas ao JeffWrite](start). Se alguma parte do arquivo não puder ser lida de jeito nenhum, o JeffWrite informa o que deixou de fora. Nesse caso, ele não salva por cima desse arquivo: Salvar pede um novo nome, para que o original preserve o que a cópia perdeu.

## Salvar um documento

Pressione **Ctrl+S** para salvar. Na primeira vez, o JeffWrite pede um nome. **Salvar Como** (F12) salva uma cópia com um novo nome. Escolha **Modelo (*.dotx)** para salvar um modelo. Um arquivo que veio de um tipo mais antigo é sempre salvo com um novo nome, para que o arquivo antigo nunca seja sobrescrito.

## Salvar uma cópia

Clique em **Arquivo > Salvar uma Cópia**. Escolha um nome. O JeffWrite grava o documento nesse arquivo, e você continua trabalhando no documento que tinha aberto. O nome dele e as alterações não salvas permanecem como estavam. Isso é útil quando você quer enviar uma versão para alguém e continuar escrevendo.

## Manter uma cópia de backup

Clique em **Arquivo > Opções** e, na página **Geral**, ative **Sempre criar cópia de backup**. Toda vez que você salvar, o JeffWrite primeiro copia o arquivo antigo para outro chamado "Backup de" seguido do nome dele, na mesma pasta. Se a cópia não puder ser feita, o JeffWrite pergunta se deve salvar sem ela. A resposta é Não, a não ser que você diga Sim, e o arquivo continua como estava. Assim você pode voltar à última versão salva.

## Salvar como modelo

Um modelo é um documento que você usa como ponto de partida. Clique em **Arquivo > Salvar como Modelo** e digite um nome. O modelo vai para **Meus Modelos**, e o documento que você está escrevendo não é alterado. Em **Salvar Como**, você também pode escolher **Modelo (*.dotx)** ou **Modelo de Texto OpenDocument (*.ott)** para salvar um arquivo de modelo onde preferir.

Para iniciar um novo documento a partir do seu modelo, clique em **Arquivo > Novo**. Seus modelos aparecem em **Meus Modelos**. Clique em um deles. Um novo documento abre com o texto e os estilos do modelo. Para remover um modelo da lista, clique com o botão direito nele e escolha **Remover de Meus Modelos**.

## Outras formas de abrir um arquivo

Na página **Abrir**, clique em **Mais Formas de Abrir**. Você também pode clicar com o botão direito em um nome da lista Recentes. Cada forma pede um arquivo ou usa aquele em que você clicou:

- **Abrir como Somente Leitura** abre o arquivo apenas para leitura. Você pode ler, rolar, pesquisar e copiar, mas não pode alterar nada. A barra de título mostra "Somente Leitura". Se você pressionar **Salvar**, o JeffWrite pede um novo nome, de modo que o arquivo aberto não é alterado.
- **Abrir como Cópia** abre um novo documento com o texto do arquivo. Ele não tem arquivo próprio, então **Salvar** pergunta onde colocá-lo. O arquivo aberto nunca é alterado.
- **Abrir em Nova Janela** abre o arquivo em uma janela própria, mesmo quando a janela em que você está estiver vazia.
- **Abrir e Reparar** serve para um arquivo .docx danificado, por exemplo um que foi interrompido ao ser copiado. Veja a seguir.

## Abrir e Reparar

Às vezes um arquivo .docx fica danificado. Uma cópia pode ter sido interrompida no meio, ou um disco pode ter apresentado erros. Um arquivo .docx é um conjunto de partes menores, como o texto, os estilos e as imagens. **Abrir e Reparar** lê todas as partes que consegue. Uma parte interrompida no meio é mantida até o ponto em que parou. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas tiverem desaparecido, o JeffWrite as recria.

O documento reparado abre como uma cópia com um nome como "Carta reparada". Uma mensagem informa o que foi mantido até uma interrupção e o que foi deixado de fora. O arquivo danificado não é alterado. Salve a cópia reparada com um nome próprio e confira o conteúdo antes de confiar nele.

## Recuperar trabalho não salvo

O JeffWrite salva uma cópia de reserva do trabalho não salvo a cada poucos minutos, em um arquivo na pasta própria dele. Se o programa fechar inesperadamente, ele oferece essas cópias na próxima vez que você o iniciar. Clique em **Arquivo > Abrir > Recuperar Trabalho Não Salvo** para vê-las depois. Você pode escolher quais abrir e salvar cada uma onde quiser.

## Imprimir

Pressione **Ctrl+P**, ou clique em **Arquivo > Imprimir**. Escolha uma impressora, o número de cópias e as páginas, veja cada página na visualização e clique em **Imprimir**. Veja [Imprimir](print).

## Criar um PDF

Clique em **Arquivo > Exportar > Exportar para PDF**. Escolha um nome. O PDF tem a mesma aparência em qualquer computador, porque as fontes ficam dentro dele.

## Relacionados

- [Boas-vindas ao JeffWrite](start)
- [Imprimir](print)
- [Usar o JeffWrite com o teclado](keyboard)
