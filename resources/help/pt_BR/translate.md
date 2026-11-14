# Traduzir texto

<!-- keywords: traduzir, tradução, tradutor, idioma, idiomas, espanhol, francês, alemão, italiano, português, holandês, russo, ucraniano, indonésio, offline, neste computador, baixar, download, modelos, substituir, inserir abaixo -->

**Revisão > Idioma > Traduzir** abre o painel **Traduzir**. Ele traduz o seu texto no seu próprio computador. Suas palavras não são enviadas para lugar nenhum, e não é preciso conexão com a internet depois que os arquivos de tradução de um idioma estão no seu computador.

## Traduzir um texto

1. Selecione o texto que você deseja traduzir. Sem nada selecionado, o painel usa o parágrafo em que está o cursor. Em uma caixa de texto selecionada, mas não em edição, ele usa todo o texto da caixa.
2. Clique em **Revisão > Idioma > Traduzir**. O painel mostra o seu texto em **Original**.
3. Em **De**, escolha o idioma do seu texto. Em **Para**, escolha o idioma desejado. O botão **Trocar** inverte os dois.
4. Clique em **Traduzir**. A tradução aparece em **Tradução**. Palavras em negrito, itálico e sublinhado continuam em negrito, itálico e sublinhado.
5. Clique em **Substituir** para colocar a tradução no lugar do seu texto, ou em **Inserir Abaixo** para adicioná-la em novos parágrafos após o seu texto. Qualquer um dos dois é uma única etapa, então **Desfazer** a desfaz.

Se você mover o cursor ou alterar o texto depois de clicar em **Traduzir**, clique em **Usar Seleção** para ler o texto novamente e, em seguida, clique em **Traduzir** outra vez. O painel não vai colocar uma tradução sobre um texto que não é mais o texto que foi traduzido.

**Cancelar** interrompe uma tradução que está demorando demais, e fechar o painel também faz isso: a janela nunca espera por ela. Textos longos levam mais tempo: uma página de texto leva alguns segundos na maioria dos computadores. O painel traduz no máximo 1.000.000 de letras de cada vez (cerca de 200 páginas); se houver mais texto selecionado, ele avisa e pede que você selecione menos. Parágrafos longos são divididos em partes de cerca de 1.500 letras, no fim de uma frase sempre que possível, e depois são juntados novamente.

## A primeira vez que você usa um idioma

Uma tradução precisa de um conjunto de arquivos, chamado modelo, para cada direção: um para inglês para alemão, outro para alemão para inglês. Cada modelo tem cerca de 37 MB. Os arquivos não fazem parte do JeffPub. Na primeira vez que você traduz com um idioma, o painel informa o que é preciso e o tamanho, por exemplo "Baixar alemão (74 MB)?". Nada é baixado até que você clique nesse botão. O painel mostra o andamento do download, e **Cancelar** interrompe o download e não deixa nada para trás.

O JeffPub verifica cada arquivo com uma impressão digital (uma soma de verificação SHA-256) antes de usá-lo. Um arquivo que não corresponde é descartado, e o idioma não é instalado.

Os arquivos ficam em uma única pasta que todos os aplicativos do JeffOffice compartilham (uma pasta chamada JeffOffice na pasta de dados do seu usuário, em Translation/models). Assim, um idioma que você baixa em um aplicativo fica pronto nos outros, e é baixado apenas uma vez. Dois idiomas que não têm modelo próprio, como alemão e francês, passam pelo inglês, então o painel precisa dos dois modelos.

## Quando um idioma não tem modelo

Se um par de idiomas não tem modelo, o painel informa isso e oferece **Abrir no Seu Navegador**. Isso abre um site gratuito de tradução, o LibreTranslate (ou o serviço cujo endereço você escolheu na configuração correspondente), com o seu texto. Só então o seu texto sai do seu computador, porque ele vai para esse site, e o painel deixa de dizer que o texto permanece no seu computador. Nada é enviado até que você clique nesse botão.

## Sobre a qualidade

Os modelos são pequenos, então funcionam rapidamente em um computador comum. Eles traduzem bem frases e pior rótulos curtos: uma única palavra ou o nome de um botão pode sair errado. Leia a tradução antes de usá-la. Nomes, números e tabulações são mantidos como estavam.

Imagens, tabulações e quebras de linha dentro de um parágrafo são preservadas. A tradução preenche apenas as palavras entre elas.

## Relacionado

- [Ortografia, dicionário de sinônimos, pesquisa e hifenização](proofing)
